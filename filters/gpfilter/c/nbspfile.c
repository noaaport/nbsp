/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <zlib.h>
#include <libgen.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include "err.h"
#include "util.h"
#include "io.h"
#include "unz.h"
#include "const.h"

#define COMPRESS_LEVEL		0
#define DUMMY_SEQNUM		90000
#define PAGE_SIZE		4000	/* default for [-z] flag */
#define LARGE_PAGE_SIZE		65536	/* default */
static char static_page_buffer[PAGE_SIZE];
static char static_large_page_buffer[LARGE_PAGE_SIZE];

struct {
  char *opt_input_fname;
  char *opt_output_fname;
  char *opt_output_dir;
  int opt_stdin;	/* [-i] */
  int opt_noheader;	/* [-t] => don't add gempak header and footer. */
  unsigned int opt_seqnum;	
  int opt_C;
  int opt_verbose;
  int opt_background;
  int opt_append;
  int opt_noccb;	  /* [-n] file saved without ccb [24 byte] header */
  int opt_compress_level;
  int opt_pagesize;
  /* variables */
  FILE *input_fp;
  FILE *output_fp;
  char *page;
  int page_size;
  char *malloc_page_buffer;
} g = {NULL, NULL, NULL, 0, 0, DUMMY_SEQNUM, 0, 0, 0, 0, 0,
       COMPRESS_LEVEL, PAGE_SIZE, NULL, NULL,
       static_large_page_buffer, LARGE_PAGE_SIZE, NULL};

static void check(void);
static int process_file(void);
static int write_cpage(FILE *fp, char *page, int page_size);
static int write_header(FILE *fp, char *page, int page_size, int *offset);
static int has_awips_line(char *fname);
static void cleanup(void);

int main(int argc, char **argv){

  int status = 0;
  int c;
  uint16_t pagesize;
  uint16_t level;
  char *optstr = "Chabd:ino:p:s:tvz:";
  char *usage = "nbspfile [-a] [-b] [-d outputdir] [-i] [-n] [-o outfile]"
    " [-p pagesize] [-s seqnum] [-t] [-v] [-z level] file [seqnum]";

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'a':
      g.opt_append = 1;
      break;
    case 'b':
      g.opt_background = 1;
      break;
    case 'C':
      g.opt_C = 1;
      break;
    case 'd':
      g.opt_output_dir = optarg;
      break;
    case 'i':
      g.opt_stdin = 1;		/* read from stdin */
      break;
    case 'n':
      g.opt_noccb = 1;		/* the file was saved without the ccb */
      break;
    case 'o':
      g.opt_output_fname = optarg;
      break;
    case 'p':
      status = strto_u16(optarg, &pagesize);
      if(status != 0)
	log_errx(1, "Illegal value for [-p].");
      else{
	g.opt_pagesize = pagesize;
      }
      break;
    case 's':
      status = strto_uint(optarg, &g.opt_seqnum);
      if(status != 0)
	log_errx(1, "Invalid sequence number.");

      break;
    case 't':
      g.opt_noheader = 1;    /* don't add gempak header and footer */
      break;
    case 'z':
      status = strto_u16(optarg, &level); 
      if((status != 0) || (level > 9))
	log_errx(1, "Invalid value for [-c].");
      else{
	g.opt_compress_level = level;
	g.page = static_page_buffer;
	g.page_size = PAGE_SIZE;	
      }
      break;
    case 'v':
      g.opt_verbose = 1;
      break;
    case 'h':
    default:
      status = 1;
      log_errx(1, usage);
      break;
    }
  }

  /*
   * The file name must be given in the argument even if the data will be
   * read from stdin. The file name is used to extract some information
   * regarding the wmo header (e.g., has_awips_line()).
   */
  if(optind == argc)
    log_errx(1, "Needs one argument.");
  
  g.opt_input_fname = argv[optind++];

  /* The (optional) sequence number */
  if(optind < argc - 1)
    log_errx(1, "Too many arguments.");
  else if(optind == argc - 1){
    status = strto_uint(argv[optind], &g.opt_seqnum);
    if(status != 0)
      log_errx(1, "Invalid sequence number.");
  }

  if(g.opt_C == 1){
    check();
    return(0);
  }

  if(g.opt_background == 1)
    set_usesyslog();

  atexit(cleanup);

  if(status == 0)
    status = process_file();

  return(status != 0 ? 1 : 0);
}

static void cleanup(void){

  if((g.input_fp != NULL) && (g.opt_stdin == 0))
    fclose(g.input_fp);

  if((g.output_fp != NULL) && (g.opt_output_fname != NULL))
    fclose(g.output_fp);

  if(g.malloc_page_buffer != NULL)
    free(g.malloc_page_buffer);
}

static int write_cpage(FILE *fp, char *page, int page_size){

  int n;
  int zip_status;
  int cpage_size;
  char *cpage = NULL;

  zip_status = zip(&cpage, &cpage_size, page, page_size, g.opt_compress_level);
  if(zip_status != 0)
     log_errx(1, "Error %d from compress function.", zip_status);

  n = write_page(fp, cpage, cpage_size);
  
  free(cpage);

  return(n);
}

static int write_header(FILE *fp, char *page, int page_size, int *offset){
  /*
   * When there is an awips line, the header consists of the first 
   * two lines of the file, but excluding the ccb that is contained in 
   * the first line. If there is no awips line (e.g., kwal_sepa40)
   * the header is the remainder of the first line.
   * "offset" is the starting position in page[] that remains to be written.
   */
  char *header;
  int header_size;
  int status = 0;
  int count = 0;
  int n;
  int ccb_size = CCB_SIZE;

  if(g.opt_noccb == 1)
    ccb_size = 0;

  if(page_size <= ccb_size){
    status = 1;
    goto end;
  }

  header = &page[ccb_size];
  header_size = 0;

  if(has_awips_line(g.opt_input_fname)){
    /*
     * The header is everything until the second '\n'
     */
    count = 2;
  }else
    count = 1;

  n = 0;
  while(n < count){
    if(header[header_size] == '\n')
      ++n;

    ++header_size;
    if(header_size + ccb_size == page_size){
      status = 1;
      goto end;
    }
  }

  *offset = header_size + ccb_size;
  if((n = write_page(fp, header, header_size)) == -1)
    status = -1;

 end:

  if(status != 0)
    log_errx(1, "Corrupt data file. No data. %s", g.opt_input_fname);

  return(status);
}

static int process_file(void){

    int nread, nwrite;
    int status = 0;
    int data_start = -1; /* just to be sure it is properly initalized later */

    if(g.opt_pagesize != g.page_size){      
      g.malloc_page_buffer = malloc(g.opt_pagesize);
      if(g.malloc_page_buffer == NULL){
	log_err(1, "Cannot process %s", g.opt_input_fname);
      }
      g.page = g.malloc_page_buffer;
      g.page_size = g.opt_pagesize;
    }

    if(g.opt_stdin == 0){
      g.input_fp = fopen_input(g.opt_input_fname);
      if(g.input_fp == NULL)
	log_err_open(g.opt_input_fname);
    }else
      g.input_fp = stdin;
    
    if(g.opt_output_dir != NULL){
      status = chdir(g.opt_output_dir);
      if(status != 0)
	log_err(1, "Cannot chdir to %s", g.opt_output_dir);
    }

    if(g.opt_output_fname != NULL){
      if(g.opt_append == 1)
	g.output_fp = fopen_output(g.opt_output_fname, "a");
      else
	g.output_fp = fopen_output(g.opt_output_fname, "w");
    }else
      g.output_fp = stdout;

    if(g.output_fp == NULL)
      log_err_open(g.opt_output_fname);

    if(g.opt_noheader == 0)
      fprintf(g.output_fp, GMPK_HEADER_FMT, (int)(g.opt_seqnum % 1000));

    nread = fread(g.page, 1, g.page_size, g.input_fp);
    if(nread > 0)
      status = write_header(g.output_fp, g.page, nread, &data_start);
    else{
      if(ferror(g.input_fp) != 0)
	log_err_read(g.opt_input_fname);
      else{
	/*
	 * g.opt_input_fname is never NULL in this program.
	 */
	log_errx(1, "Corrupt data file. No data. %s", g.opt_input_fname);
      }
    }

    if(status != 0)
      return(1);

    if(g.opt_compress_level > 0){
	if((g.opt_verbose == 1) && (g.opt_output_fname != NULL)){
	  log_info("Compressing %s", g.opt_output_fname);
	}

	/*
	 * For gempak compatibility, the entire product must be split
	 * in 4000 bytes frames (as in the raw noaaport), then compress
	 * each frame individually, and then catenate the compressed
	 * frames. That is prepended with the wmo and awips header 
	 * (from the the first and second lines of the original uncompressed
	 * file). Furthermore, the frames must be compressed with level 9.
	 */
	while(nread > 0){
	  nwrite = write_cpage(g.output_fp, g.page, nread);
	  nread = read_page(g.input_fp, g.page, g.page_size);
	}
    }else{
      while(nread > 0){
	nwrite = write_page(g.output_fp, &g.page[data_start], 
			    nread - data_start);
	nread = read_page(g.input_fp, g.page, g.page_size);
	data_start = 0;
      }	  
    }

    if(g.opt_noheader == 0)
      fprintf(g.output_fp, GMPK_TRAILER_STR);

    if((g.opt_verbose == 1) && (g.opt_output_fname != NULL)){
	log_info("Archiving %s in %s", g.opt_output_fname, g.opt_output_dir);
    }

  return(0);
}

static void check(void){

  if(g.opt_output_fname != NULL)
     fprintf(stdout, "outputfile: %s\n", g.opt_output_fname);

  fprintf(stdout, "background: %d\n", g.opt_background);
  fprintf(stdout, "append: %d\n", g.opt_append);
  fprintf(stdout, "level: %d\n", g.opt_compress_level);
  fprintf(stdout, "pagesize: %d\n", g.opt_pagesize);
  fprintf(stdout, "seqnum: %u\n", g.opt_seqnum);
}

static int has_awips_line(char *fname){

  char *s;

  s = strchr(fname, FNAME_AWIPS_SEP_CHAR);
  if(s != NULL)
    return(1);

  return(0);
}

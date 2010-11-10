/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
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
#include <stdint.h>
#include <sys/stat.h>
#include "err.h"
#include "util.h"
#include "io.h"
#include "unz.h"

#define COMPRESS_LEVEL		9

struct {
  char *opt_input_fname;
  char *opt_output_fname;
  int opt_C;
  int opt_background;
  int opt_compress_level;
  /* variables */
  FILE *input_fp;
  FILE *output_fp;
  char *buffer;
  int buffer_size;
} g = {NULL, NULL, 0, 0, COMPRESS_LEVEL, NULL, NULL, NULL, 0};

static int filesize(char *fname);
static void check(void);
static int process_file(void);
static void cleanup(void);

int main(int argc, char **argv){

  int status = 0;
  int c;
  uint16_t level;
  char *optstr = "Chbc:o:";
  char *usage = "nbspz [-b] [-c level] [-o outfile] [file]";

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'c':
      status = strto_u16(optarg, &level); 
      if((status != 0) || (level > 9))
	log_errx(1, "Illegal value for [-c].");
      else{
	g.opt_compress_level = level;
      }
      break;
    case 'C':
      g.opt_C = 1;
      break;
    case 'o':
      g.opt_output_fname = optarg;
      break;
    case 'h':
    default:
      status = 1;
      log_errx(1, usage);
      break;
    }
  }

  if(optind < argc - 1)
    log_errx(1, "Too many arguments.");
  else if(optind == argc - 1)
    g.opt_input_fname = argv[optind++];
  else
    log_errx(1, "Needs one argument.");

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

  if(g.input_fp != NULL)
    fclose(g.input_fp);

  if((g.output_fp != NULL) && (g.opt_output_fname != NULL))
    fclose(g.output_fp);

  if(g.buffer != NULL)
    free(g.buffer);
}

static int filesize(char *fname){

  struct stat sb;

  if(stat(fname, &sb) == -1)
    return(-1);

  return(sb.st_size);
}

static int write_cpage(FILE *fp, char *page, int page_size){

  int status = 0;
  int zip_status;
  int cpage_size;
  char *cpage = NULL;

  zip_status = zip(&cpage, &cpage_size, page, page_size, g.opt_compress_level);
  if(zip_status != 0)
     log_errx(1, "Error %d from compress function.", zip_status);

  status = write_page(fp, cpage, cpage_size);
  
  free(cpage);

  return(status);
}

static int process_file(void){

  int status = 0;

  g.buffer_size = filesize(g.opt_input_fname);
  g.buffer = malloc(g.buffer_size);
  if(g.buffer == NULL)
    log_err(1, "Cannot process %s", g.opt_input_fname);
  
  g.input_fp = fopen_input(g.opt_input_fname);
  if(g.input_fp == NULL)
    log_err_open(g.opt_input_fname);
  
  if(g.opt_output_fname != NULL){
    g.output_fp = fopen_output(g.opt_output_fname, "w");
  }else
    g.output_fp = stdout;
  
  if(g.output_fp == NULL)
    log_err_open(g.opt_output_fname);
  
  status = read_page(g.input_fp, g.buffer, g.buffer_size);

  if(status == 0){
    if(g.opt_compress_level > 0)
	status = write_cpage(g.output_fp, g.buffer, g.buffer_size);
    else
      status = write_page(g.output_fp, g.buffer, g.buffer_size);
  }

  return(status);
}

static void check(void){

  fprintf(stdout, "outputfile: %s\n", g.opt_output_fname);
  fprintf(stdout, "background: %d\n", g.opt_background);
  fprintf(stdout, "level: %d\n", g.opt_compress_level);
}

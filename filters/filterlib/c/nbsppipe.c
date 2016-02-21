/*
 * Copyright (c) 2005-2016 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <libgen.h>
#include <fcntl.h>
#include <string.h>
#include "err.h"
#include "util.h"
#include "io.h"
#include "const.h"

#define DUMMY_SEQNUM		90000
#define PAGE_SIZE		65536

struct {
  char *opt_input_fname;
  char *opt_output_fname;
  char *opt_output_dir;
  int opt_background;
  unsigned int opt_seqnum;
  int opt_append;	/* [-a] outputfile in append mode */
  int opt_noccb;	/* [-n] file saved without ccb [24 byte] header */
  int opt_header;	/* [-g] add gempak-like header and trailer */
  int opt_ccbsize;      /* [-c] specify a different (ccb) size to cut */
  /* variables */
  FILE *input_fp;
  FILE *output_fp;
  char page[PAGE_SIZE];
  int page_size;
} g = {NULL, NULL, NULL, 0, DUMMY_SEQNUM, 0, 0, 0, CCB_SIZE, NULL, NULL,
       {0}, PAGE_SIZE};

static int process_file(void);
static void cleanup(void);

int main(int argc, char **argv){

  int status = 0;
  int c;
  unsigned int seqnum;
  uint16_t ccbsize;
  char *optstr = "habc:d:gno:s:";
  char *usage = "nbsppipe [-h] [-b] [-c ccbsize] [-d directory] [-g] [-n]"
    " [-o output [-a]] [-s N] file | < file";

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'a':
      g.opt_append = 1;
      break;
    case 'b':
      g.opt_background = 1;
      break;
    case 'c':
      status = strto_u16(optarg, &ccbsize);
      if(status != 0)
	log_errx(1, "Illegal value for [-c].");
      else{
	g.opt_ccbsize = ccbsize;
      }
      break;
    case 'd':
      g.opt_output_dir = optarg;
      break;
    case 'g':
      g.opt_header = 1;		/* add header and trailer */
      break;
    case 'n':
      g.opt_noccb = 1;		/* the file was saved without the ccb */
      break;
    case 'o':
      g.opt_output_fname = optarg;
      break;
    case 's':
      status = strto_uint(optarg, &seqnum);
      if(status != 0)
	log_errx(1, "Invalid value for [-s].");
      else{
	g.opt_seqnum = seqnum;
      }
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
  else if(optind == argc -1)
    g.opt_input_fname = argv[optind++];

  if(g.opt_background == 1)
    set_usesyslog();

  atexit(cleanup);

  if(status == 0)
    status = process_file();

  return(status != 0 ? 1 : 0);
}

static void cleanup(void){

  if((g.input_fp != NULL) && (g.opt_input_fname != NULL))
    fclose(g.input_fp);

  if((g.output_fp != NULL) && (g.opt_output_fname != NULL))
    fclose(g.output_fp);
}

static int process_file(void){

  int ccb_size = g.opt_ccbsize;
  int nread, nwrite;
  int status = 0;
  int data_start;

  if(g.opt_input_fname != NULL){
    g.input_fp = fopen_input(g.opt_input_fname);
    if(g.input_fp == NULL)
      log_err_open(g.opt_input_fname);
  }else
    g.input_fp = stdin;

  /*
   * If the outout dir is specified, we must change to it but after
   * opening the input file.
   */
  if(g.opt_output_dir != NULL){
    status = chdir(g.opt_output_dir);
    if(status != 0)
      log_err(1, "Cannot chdir to %s", g.opt_output_dir);
  }

  if(g.opt_output_fname != NULL){
    if(g.opt_append == 0)
      g.output_fp = fopen_output(g.opt_output_fname, "w");
    else
      g.output_fp = fopen_output(g.opt_output_fname, "a");
    
    if(g.output_fp == NULL)
      log_err_open(g.opt_output_fname);
  }else
    g.output_fp = stdout;

  if(g.opt_noccb == 1)
    ccb_size = 0;

  if(g.opt_header == 1)
    fprintf(g.output_fp, GMPK_HEADER_FMT, (int)(g.opt_seqnum % 1000));
  
  nread = read_page(g.input_fp, g.page, g.page_size);
  
  if(nread <= ccb_size){
    status = 1;
    log_errx(1, "Corrupted data file.");
  }
    
  data_start = ccb_size;

  while(nread > 0){
    nwrite = write_page(g.output_fp, &g.page[data_start], nread - data_start);

    /*
     * write_page() handles the error already (exits the program)
     */
    if(nwrite == -1)
      log_err(1, "%s", "write_page() error");
    
    nread = read_page(g.input_fp, g.page, g.page_size);
    data_start = 0;
  }	  

  if(g.opt_header == 1)
    fprintf(g.output_fp, GMPK_TRAILER_STR);

  return(0);
}

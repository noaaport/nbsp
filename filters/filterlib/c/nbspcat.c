/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
/*
 * This tool provides the essential functionality of nbspfile and nbsppipe,
 * but it takes the command line arguments from stdin. The purpose is to
 * be able to "|open" it once, from a tcl script, and then write the
 * commands to the file descriptor. The program is invoked as
 *
 *   char *usage = "nbspcat [-h] [-b]";
 *
 * and each command line must have the syntax
 *
 *   char *usage = "nbspcat [-g] [-n] [-o output [-a]] [-s N] file";
 *
 * The options are similar to those in nbsppipe.
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
#include "strsplit.h"
#include "config.h"	/* optreset */

#define DUMMY_SEQNUM		90000
#define PAGE_SIZE		65536
#define CMD_LINE_BUFFER_SIZE	1024

struct {
  int opt_background;
  char *opt_input_fname;
  char *opt_output_fname;
  unsigned int opt_seqnum;
  int opt_append;	/* [-a] outputfile in append mode */
  int opt_noccb;	/* [-n] file saved without ccb [24 byte] header */
  int opt_header;	/* [-g] add gempak-like header and trailer */
  /* variables */
  FILE *input_fp;
  FILE *output_fp;
  struct strsplit_st *strp;  
  char page[PAGE_SIZE];
  int page_size;
} g = {0, NULL, NULL, DUMMY_SEQNUM, 0, 0, 0, NULL, NULL, NULL,
       {0}, PAGE_SIZE};

static void process_input(void);
static void process_cmd(int argc, char **argv);
static void process_file(void);
static void close_files(void);
static void cleanup(void);

int main(int argc, char **argv){

  char *optstr = "bh";
  char *usage = "nbspcat [-h] [-b]";
  int c;
  int status = 0;

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'h':
    default:
      status = 1;
      log_errx(1, usage);
      break;
    }
  }

  if(optind <= argc - 1)
    log_errx(1, "Too many arguments.");

  if(g.opt_background == 1)
    set_usesyslog();

  atexit(cleanup);

  process_input();

  return(0);
}

static void process_input(void){

  char line[CMD_LINE_BUFFER_SIZE];
  int line_max_size = CMD_LINE_BUFFER_SIZE;
  int line_size;
  int lineno = 0;
  int status = 0;

  while(fgets(line, line_max_size, stdin) != NULL){
    ++lineno;
    line_size = strlen(line);
    if(line[line_size - 1] != '\n'){
      log_errx(1, "Line %d exceeds maximum size %d.", lineno, line_max_size);
      status = 1;
      break;
    }
    line[line_size - 1] = '\0';
    g.strp = strsplit_recreate(line, " ", STRSPLIT_FLAG_IGNEMPTY, g.strp);
    if(g.strp == NULL)
      log_err(1, "Error in strplit_create()");

    process_cmd(g.strp->argc, g.strp->argv);
  }
}

static void process_cmd(int argc, char **argv){

  int status = 0;
  int c;
  unsigned int seqnum;
  char *optstr = "abgno:s:";
  char *usage = "nbspcat [-g] [-n] [-o output [-a]] [-s N] file";

  /* Reset defauts */
  g.opt_input_fname = NULL;
  g.opt_output_fname = NULL;
  g.opt_seqnum = DUMMY_SEQNUM;
  g.opt_append = 0;
  g.opt_noccb = 0;
  g.opt_header = 0;

  optind = 1;
#ifdef HAVE_OPTRESET
  optreset = 1;
#endif

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'a':
      g.opt_append = 1;
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
	log_warnx("Invalid value for [-s].");
      else{
	g.opt_seqnum = seqnum;
      }
      break;
    case 'h':
    default:
      status = 1;
      log_warnx(usage);
      break;
    }
  }

  if(status != 0)
    return;

  if(optind < argc - 1){
    status = 1;
    log_warnx("Too many arguments.");
  }else if(optind == argc){
    status = 1;
    log_warnx(usage);
  }else
    g.opt_input_fname = argv[optind];

  if(status == 0){
    process_file();
    close_files();
  }
}
static void cleanup(void){

  close_files();
  strsplit_delete(g.strp);
}

static void close_files(void){

  if(g.input_fp != NULL){
    fclose(g.input_fp);
    g.input_fp = NULL;
  }

  if((g.output_fp != NULL) && (g.opt_output_fname != NULL)){
    fclose(g.output_fp);
    g.output_fp = NULL;
  }
}

static void process_file(void){

  int ccb_size = CCB_SIZE;
  int nread, nwrite;
  int data_start;

  g.input_fp = fopen_input(g.opt_input_fname);
  if(g.input_fp == NULL){
    log_errn_open(g.opt_input_fname);
    return;
  }

  if(g.opt_output_fname != NULL){
    if(g.opt_append == 0)
      g.output_fp = fopen_output(g.opt_output_fname, "w");
    else
      g.output_fp = fopen_output(g.opt_output_fname, "a");
    
    if(g.output_fp == NULL){
      log_errn_open(g.opt_output_fname);
      return;
    }
  }else
    g.output_fp = stdout;

  if(g.opt_noccb == 1)
    ccb_size = 0;

  if(g.opt_header == 1)
    fprintf(g.output_fp, GMPK_HEADER_FMT, (int)(g.opt_seqnum % 1000));
  
  nread = read_page(g.input_fp, g.page, g.page_size);
  
  if(nread <= ccb_size){
    log_warnx("Corrupted data file.");
    return;
  }
    
  data_start = ccb_size;

  while(nread > 0){
    nwrite = write_page(g.output_fp, &g.page[data_start], nread - data_start);
    nread = read_page(g.input_fp, g.page, g.page_size);
    data_start = 0;
  }	  

  if(g.opt_header == 1)
    fprintf(g.output_fp, GMPK_TRAILER_STR);
}

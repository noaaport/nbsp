/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>
#include "err.h"
#include "util.h"

/*
 * NOTE: This program is not used. It is here mainly for documentation
 * purposes and as a placeholder.
 */

/*
 * Usage: nbsprad2info <file> | < <file>
 *
 * The program reads from a file or stdin, but the data must start with the
 * 24 byte ldm header. The description of that header is in
 *
 * 2620010D_Final_052708.pdf
 *
 * The typical usage is therefore
 *
 * nbsprad2info TJUA_20100306_1712
 */

#define L2_HEADER_SIZE 24

struct l2_header_st {
  unsigned char header[L2_HEADER_SIZE];
  char tape_filename[10];
  char extension_number[4];
  unsigned int julianday;
  unsigned int milliseconds;
  char ICAO[5];
  char icao[5];
  /* Calculated */
  unsigned int seconds;
};

struct {
  int opt_background;	/* -b */
  int opt_skipcount;	/* -c <count> => skip the first <count> bytes */
  int opt_timeonly;	/* -t => only print the time (unix secs) and icao */
  char *opt_inputfile;
  /* variables */
  int fd;
} g = {0, 0, 0, NULL, -1};

static int process_file(void);
static void cleanup(void);

static void cleanup(void){

  if((g.fd != -1) && (g.opt_inputfile != NULL))
    (void)close(g.fd);
}

int main(int argc, char **argv){

  char *optstr = "bc:t";
  char *usage = "nbsprad2info [-b] [-c <count>] [-t] <file> | < file";
  int status = 0;
  int c;

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'c':
      status = strto_int(optarg, &g.opt_skipcount);
      if((status == 1) || (g.opt_skipcount <= 0)){
	log_errx(1, "Invalid argument to [-c] option.");
      }
      break;
    case 't':
      g.opt_timeonly = 1;
      break;
    case 'h':
    default:
      log_info(usage);
      exit(0);
      break;
    }
  }

  if(g.opt_background == 1)
    set_usesyslog();

  if(optind < argc - 1)
    log_errx(1, "Too many arguments.");
  else if(optind == argc -1)
    g.opt_inputfile = argv[optind++];

  atexit(cleanup);
  status = process_file();

  return(status != 0 ? 1 : 0);
}

int process_file(void){

  int fd;
  int n;
  unsigned char *b;
  struct l2_header_st header;
  char dummy[4096];
  size_t dummy_size = 4096;
  size_t nleft;
  size_t ndummy_read;
  char *p, *q;

  memset(&header, 0, sizeof(struct l2_header_st));

  if(g.opt_inputfile == NULL)
    fd = fileno(stdin);
  else{
    fd = open(g.opt_inputfile, O_RDONLY);
    if(fd == -1)
      log_err_open(g.opt_inputfile);
    else
      g.fd = fd;
  }

  b = &header.header[0];

  if(g.opt_skipcount != 0){
    nleft = g.opt_skipcount;
    while(nleft > 0){
      ndummy_read = nleft;
      if((size_t)nleft > dummy_size)
	ndummy_read = dummy_size;

      if(read(fd, dummy, ndummy_read) == -1)
	log_err(1, "Error from read()");

      nleft -= ndummy_read;
    }
  }

  n = read(fd, b, L2_HEADER_SIZE);
  if(n == -1)
    log_err(1, "Error from read()");
  else if(n < L2_HEADER_SIZE)
    log_errx(1, "Corrupt file.");

  memcpy(&header.tape_filename, b, 8);	/* discard the period */
  memcpy(&header.extension_number, &b[9], 3);
  header.julianday = (b[12] << 24) + (b[13] << 16) + (b[14] << 8) + b[15];
  header.milliseconds = (b[16] << 24) + (b[17] << 16) + (b[18] << 8) + b[19];
  memcpy(header.ICAO, &b[20], 4);

  p = &header.ICAO[0];
  q = &header.icao[0];
  while(*p != '\0'){
    *q++ = tolower(*p++);
  }

  header.seconds = (header.julianday - 1)*24*3600 + header.milliseconds/1000;

  if(g.opt_timeonly)
    fprintf(stdout, "%u %s", header.seconds, header.icao);
  else
    fprintf(stdout, "%s %s %u %u %u %s",
	    header.tape_filename, header.extension_number,
	    header.julianday, header.milliseconds,
	    header.seconds, header.icao);
  /*
   * If reading from stdin, then consume the input to avoid generating
   * a pipe error in the tcl scripts.
   * nbspunz should be called with the [-n] option.
   */
  if(g.opt_inputfile == NULL){
    while(read(fd, dummy, dummy_size) > 0)
      ;
  }

  return(0);
}

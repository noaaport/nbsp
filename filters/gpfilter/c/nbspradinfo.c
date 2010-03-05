/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
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
#include "err.h"
#include "util.h"

/*
 * Usage: nbspradinfo <file> | < <file>
 *
 * The program reads from a file or stdin, but the data must start with the
 * nids header (i.e., the ccb and wmo headers must have been removed;
 * but see below).
 *
 * The typical usage is therefore
 *
 * nbspunz -c 54 ksjt_sdus54-n0rsjt.020131_16452648 | nbspradinfo
 * nbspunz -c 54 n0rsjt_20091002_0007.nids | nbspradinfo
 *  
 * In the first case the data file is one from the spool directory;
 * in the second the the data file is on from the digatmos/nexrad directory.
 *
 * If the data does not start with nids header, the [-c <count>] options
 * can be used to instruct the program to ignore the first <count> bytes.
 * So an alternative usage is
 *
 * nbspunz ksjt_sdus54-n0rsjt.020131_16452648 | nbspradinfo -c 54
 *
 * or, if the file is not compressed
 *
 * nbspradinfo -c 54 koun_sdus54-n0qvnx.210224_204899761
 * nbspradinfo -c 41 n0qvnx_20100221_0224.nids
 *               (41 = 30 + gempak header [const.h])
 *
 * The default information printed is
 *
 *          nheader.pdb_lat, nheader.pdb_lon, nheader.pdb_height, seconds,
 *          nheader.pdb_mode, nheader.pdb_code
 *
 * unless [-t] is given in which case only the "seconds" is printed.
 */
#define NIDS_HEADER_SIZE 120	/* message and pdb */

struct nids_header_st {
  unsigned char header[NIDS_HEADER_SIZE];
  int m_code;
  int m_days;
  unsigned int m_seconds;
  unsigned int m_length;	/* unused */
  int m_source;                 /* unused */
  int m_destination;            /* unused */
  int m_numblocks;              /* unused */
  float pdb_lat;
  float pdb_lon;
  int pdb_height;
  int pdb_code;
  int pdb_mode;
};

struct {
  int opt_background;	/* -b */
  int opt_skipcount;	/* -c <count> => skip the first <count> bytes */
  int opt_timeonly;	/* -t => only extract and print the time (unix secs) */
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
  char *usage = "nbspradinfo [-b] [-c <count>] [-t] <file> | < file";
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
  unsigned int seconds;
  unsigned char *b;
  struct nids_header_st nheader;
  int lat, lon;
  char dummy[4096];
  size_t dummy_size = 4096;
  size_t nleft;
  size_t ndummy_read;

  memset(&nheader, 0, sizeof(struct nids_header_st));

  if(g.opt_inputfile == NULL)
    fd = fileno(stdin);
  else{
    fd = open(g.opt_inputfile, O_RDONLY);
    if(fd == -1)
      log_err_open(g.opt_inputfile);
    else
      g.fd = fd;
  }

  b = &nheader.header[0];

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

  n = read(fd, b, NIDS_HEADER_SIZE);
  if(n == -1)
    log_err(1, "Error from read()");
  else if(n < NIDS_HEADER_SIZE)
    log_errx(1, "Corrupt file.");

  nheader.m_code = (b[0] << 8) + b[1];
  nheader.m_days = (b[2] << 8) + b[3] - 1;
  nheader.m_seconds = (b[4] << 24) + (b[5] << 16) + (b[6] << 8) + b[7];

  lat = (b[20]<< 24) + (b[21] << 16) + (b[22] << 8) + b[23];
  lon = (b[24]<< 24) + (b[25] << 16) + (b[26] << 8) + b[27];
  nheader.pdb_lat = ((float)lat)/1000;
  nheader.pdb_lon = ((float)lon)/1000;

  nheader.pdb_height = (b[28] << 8) + b[29];
  nheader.pdb_code = (b[30] << 8) + b[31];    /* same as m_code */
  nheader.pdb_mode = (b[32] << 8) + b[33];

  seconds = nheader.m_days * 24 * 3600 + nheader.m_seconds;

  if(g.opt_timeonly)
    fprintf(stdout, "%u", seconds);
  else
    fprintf(stdout, "%.3f %.3f %d %u %d %d",
	    nheader.pdb_lat, nheader.pdb_lon, nheader.pdb_height, seconds,
	    nheader.pdb_mode, nheader.pdb_code);

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

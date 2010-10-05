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
 * Usage: nbspradinfo [-c <count>] <file> | < <file>
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
 * in the second, the data file is from the digatmos/nexrad directory.
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
 * If the file does not have the gempak header (as the tmp file used
 * by the rstfilter.lib), then
 *
 * nbspradinfo -c 30 n0qvnx_20100221_0224.tmp
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
  unsigned int m_msglength;	/* unused */
  int m_source;                 /* unused */
  int m_destination;            /* unused */
  int m_numblocks;              /* unused */
  int pdb_lat;
  int pdb_lon;
  int pdb_height;
  int pdb_code;
  int pdb_mode;
  /* derived values */
  unsigned int unixseconds;
  float lat;
  float lon;
};

struct {
  int opt_background;	/* -b */
  int opt_skipcount;	/* -c <count> => skip the first <count> bytes */
  int opt_timeonly;	/* -t => only extract and print the time (unix secs) */
  char *opt_inputfile;
  /* variables */
  int fd;
} g = {0, 0, 0, NULL, -1};

/* general functions */
static int process_file(void);
static void cleanup(void);
/* extraction functions */
static uint16_t extract_uint16(unsigned char *p, int halfwordid);
static uint32_t extract_uint32(unsigned char *p, int halfwordid);
static int extract_int32(unsigned char *p, int halfwordid);

/* decoding functions */
static void decode_nids_header(struct nids_header_st *nheader);

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
  unsigned char *b;
  struct nids_header_st nheader;
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

  decode_nids_header(&nheader);

  if(g.opt_timeonly)
    fprintf(stdout, "%u", nheader.unixseconds);
  else
    fprintf(stdout, "%.3f %.3f %d %u %d %d",
	    nheader.lat, nheader.lon, nheader.pdb_height,
	    nheader.unixseconds,
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

/*
 * decoding functions
 */
static void decode_nids_header(struct nids_header_st *nheader){

  unsigned char *b = nheader->header;

  nheader->m_code = extract_uint16(b, 1);
  nheader->m_days = extract_uint16(b, 2) - 1;
  nheader->m_seconds = extract_uint32(b, 3);

  /* msglength is the file length without headers or trailers */
  nheader->m_msglength = extract_uint32(b, 5); 
  nheader->m_source = extract_uint16(b, 7);
  nheader->m_destination = extract_uint16(b, 8);
  nheader->m_numblocks = extract_uint16(b, 9);

  nheader->pdb_lat = extract_int32(b, 11);
  nheader->pdb_lon = extract_int32(b, 13);

  nheader->pdb_height = extract_uint16(b, 15);
  nheader->pdb_code = extract_uint16(b, 16);    /* same as m_code */
  nheader->pdb_mode = extract_uint16(b, 17);

  /* derived */
  nheader->lat = ((float)nheader->pdb_lat)/1000.0;
  nheader->lon = ((float)nheader->pdb_lon)/1000.0;
  nheader->unixseconds = nheader->m_days * 24 * 3600 + nheader->m_seconds;
}

/*
 * extraction functions
 */
static uint16_t extract_uint16(unsigned char *p, int halfwordid){
  /*
   * The halfwordid argument is the (1-based) index number as in the Unisys
   * documentation.
   */
  uint16_t r;	/* result */
  unsigned char *b = p;

  b += (halfwordid - 1) * 2;
  r = (b[0] << 8) + b[1];  
  
  return(r);
}

static uint32_t extract_uint32(unsigned char *p, int halfwordid){

  uint32_t r;	/* result */
  unsigned char *b = p;

  b += (halfwordid - 1) * 2;
  r = (b[0] << 24) + (b[1] << 16) + (b[2] << 8) + b[3];

  return(r);
}

static int extract_int32(unsigned char *p, int halfwordid){

  int r;	/* result */
  unsigned char *b = p;

  b += (halfwordid - 1) * 2;
  r = (b[0] << 24) + (b[1] << 16) + (b[2] << 8) + b[3];

  return(r);
}

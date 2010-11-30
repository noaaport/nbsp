/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id: dcnids.c 792 2010-11-30 02:29:34Z nieves $
 */
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <inttypes.h>
#include "const.h"
#include "err.h"
#include "misc.h"
#include "util.h"
#include "dcnids_extract.h"
#include "dcnids_header.h"

/*
 * Usage: nbspradinfo [-b] [-c <count> | -C] [-l] [-t] <file> | < <file>
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
 * or, if the files have an uncompressed nids header
 *
 * nbspradinfo -c 54 tjsj_sdus52-n0qjua.152011_99050818
 * nbspradinfo -c 41 n0qjua_20101015_1936.nids
 *               (41 = 30 + gempak header [const.h])
 *
 * If the file does not have the gempak header (as the tmp file used
 * by the rstfilter.lib and the nids files saved by the gisfilter), then
 *
 * nbspradinfo -c 30 n0qvnx_20100221_0224.tmp
 * nbspradinfo -C n0qvnx_20100221_0224.tmp
 *
 * The default information printed is
 *
 *          nheader.pdb_lat, nheader.pdb_lon, nheader.pdb_height, seconds,
 *          nheader.pdb_mode, nheader.pdb_code
 *
 * If [-t] is given then only the "seconds" is printed, and if [-l] is given
 * then the m_msglength is also printed.
 */

struct {
  char *opt_inputfile;
  int opt_background;	/* -b */
  int opt_skipcount;	/* -c <count> => skip the first <count> bytes */
  int opt_skipwmoawips; /* -C => skip wmo + awips header (30 bytes) */
  int opt_timeonly;	/* -t => only extract and print the time (unix secs) */
  int opt_lengthonly;	/* -l => only extract and print the m_msglength */
  /* variables */
  int fd;
} g = {NULL, 0, 0, 0, 0, 0, -1};

/* general functions */
static int process_file(void);
static void cleanup(void);

static void cleanup(void){

  if((g.fd != -1) && (g.opt_inputfile != NULL))
    (void)close(g.fd);
}

int main(int argc, char **argv){

  char *optstr = "bCltc:";
  char *usage = "nbspradinfo [-b] [-c <count> | -C] [-l] [-t] <file> | < file";
  int status = 0;
  int c;
  int opt_cC = 0;	/* c and C together is a conflict */

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'l':
      g.opt_lengthonly = 1;
      break;
    case 't':
      g.opt_timeonly = 1;
      break;
    case 'C':
      ++opt_cC;
      g.opt_skipwmoawips = 1;  /* not used further */
      g.opt_skipcount = WMOAWIPS_HEADER_SIZE;
      break;
    case 'c':
      ++opt_cC;
      status = strto_int(optarg, &g.opt_skipcount);
      if((status == 1) || (g.opt_skipcount <= 0)){
	log_errx(1, "Invalid argument to [-c] option.");
      }
      break;
    default:
      log_info(usage);
      exit(0);
      break;
    }
  }

  if(opt_cC >= 2)
    log_errx(1, "Invalid combination of options: c and C.");

  if(g.opt_background == 1)
    set_usesyslog();

  if(optind < argc - 1)
    log_errx(1, "Too many arguments.");
  else if(optind == argc - 1)
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
  char skipbuffer[4096];
  size_t skipbuffer_size = 4096;

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
    n = read_skip_count(fd, g.opt_skipcount,
			skipbuffer, skipbuffer_size);
    if(n == -1)
      log_err(1, "Error from read_skip_count()");
    else if(n != 0)
      log_err(1, "Error from read_skip_count(). Short file.");
  }

  n = read(fd, b, NIDS_HEADER_SIZE);
  if(n == -1)
    log_err(1, "Error from read()");
  else if(n < NIDS_HEADER_SIZE)
    log_errx(1, "Corrupt file.");

  dcnids_decode_header(&nheader);

  if((g.opt_timeonly != 0) && (g.opt_lengthonly != 0))
    fprintf(stdout, "%" PRIuMAX " %u",
	    (uintmax_t)nheader.unixseconds,
	    nheader.m_msglength);
  else if(g.opt_timeonly != 0)
    fprintf(stdout, "%" PRIuMAX, (uintmax_t)nheader.unixseconds);
  else if(g.opt_lengthonly != 0)
    fprintf(stdout, "%u", nheader.m_msglength);
  else{
    fprintf(stdout, "%.3f %.3f %d " "%" PRIuMAX " %d %d",
	    nheader.lat,
	    nheader.lon,
	    nheader.pdb_height,
	    (uintmax_t)nheader.unixseconds,
	    nheader.pdb_mode,
	    nheader.pdb_code);
  }

  /*
   * If reading from stdin, then consume the input to avoid generating
   * a pipe error in the tcl scripts.
   * nbspunz should be called with the [-n] option.
   */
  if(g.opt_inputfile == NULL){
    while(read(fd, skipbuffer, skipbuffer_size) > 0)
      ;
  }

  return(0);
}

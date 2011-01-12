/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

/*
 * Usage: nbspradinfo [-b] [-c <count> | -C] [-l] [-t] <file> | < <file>
 *
 * The program reads from a file or stdin, but in either case the file
 * must be the uncompressed file, including the wmo header, but with the
 * ccb header removed (If the file has the gempak header, it will be
 * detected and it will be skiped.)
 *
 * The typical usage is therefore
 *
 * nbspunz -c 24 ksjt_sdus54-n0rsjt.020131_16452648 | nbspradinfo
 * nbspunz -c 24 n0rsjt_20091002_0007.nids | nbspradinfo
 *  
 * In the first case the data file is one from the spool directory;
 * in the second, the data file is from the digatmos/nexrad directory.
 *
 * The same effect is achieved by
 *
 * nbspunz -C ksjt_sdus54-n0rsjt.020131_16452648 | nbspradinfo
 * nbspunz -C n0rsjt_20091002_0007.nids | nbspradinfo
 *
 * If the data does not start with the wmo header, the [-c <count>] options
 * can be used to instruct the program to ignore the first <count> bytes.
 * So an alternative usage is
 *
 * nbspunz ksjt_sdus54-n0rsjt.020131_16452648 | nbspradinfo -c 24
 * nbspunz ksjt_sdus54-n0rsjt.020131_16452648 | nbspradinfo -C
 * nbspunz n0rsjt_20091002_0007.nids | nbspradinfo -C
 *
 * If the files have an uncompressed nids header
 *
 * nbspradinfo -c 24 tjsj_sdus52-n0qjua.152011_99050818  (from the spooldir)
 * nbspradinfo -C tjsj_sdus52-n0qjua.152011_99050818
 * nbspradinfo n0qjua_20101015_1936.nids (data dirs)
 *
 * The default information printed is
 *
 *          nheader.pdb_lat, nheader.pdb_lon, nheader.pdb_height, seconds,
 *          nheader.pdb_mode, nheader.pdb_code
 *
 * If [-t] is given then only the "seconds" is printed, and if [-l] is given
 * then the m_msglength is also printed.
 */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <inttypes.h>
#include <ctype.h>
#include "const.h"
#include "err.h"
#include "misc.h"
#include "util.h"
#include "dcnids_extract.h"
#include "dcnids_header.h"
#include "dcnids_misc.h"

struct {
  char *opt_inputfile;
  int opt_background;	/* -b */
  int opt_skipcount;	/* -c <count> => skip the first <count> bytes */
  int opt_timeonly;	/* -t => only extract and print the time (unix secs) */
  int opt_lengthonly;	/* -l => only extract and print the m_msglength */
  /* variables */
  int fd;
} g = {NULL, 0, 0, 0, 0, -1};

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
      g.opt_skipcount = CCB_SIZE;
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

  if(opt_cC > 1)
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
  struct nids_header_st nheader;
  int status;
  char skipbuffer[4096];
  size_t skipbuffer_size = 4096;

  if(g.opt_inputfile == NULL)
    fd = fileno(stdin);
  else{
    fd = open(g.opt_inputfile, O_RDONLY);
    if(fd == -1)
      log_err_open(g.opt_inputfile);
    else
      g.fd = fd;
  }

  status = dcnids_read_header(fd, g.opt_skipcount, &nheader);
  if(status != 0){
    if(g.opt_inputfile != NULL)
      log_warnx("Error reading nids header from %s", g.opt_inputfile);
  
    if(status == -1)
      log_err(1, "Error reading file.");
    else
      log_errx(1, "Error reading file. Short file.");
  }

  if(dcnids_verify_wmoawips_header(nheader.buffer) != 0)
    log_errx(1, "Invalid wmo header.");

  if(dcnids_decode_header(&nheader) != 0)
    log_errx(1, "Invalid pdb header; maybe a zlib compressed header.");

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

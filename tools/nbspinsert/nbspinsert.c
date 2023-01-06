/*
 * Copyright (c) 2023 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 *
 * Usage: nbspinsert -i [-f nbspinfifo] finfo < file
 *        nbspinsert [-f nbspinfifo] finfo
 *        nbspinsert [-f nbspinfifo] < file_with_finfolist
 * where
 *
 * finfo = seq type cat code npchidx fname fpath
 *
 * NOTE: The following comments are borrowed from the original nbspinsert.tcl
 * script (in nntpfilter) with appropriate revisions.
 *
 * In the first form, it will save the data in fpath, and then process the
 * file. In the second form, it assumes that the data is already in fpath.
 * The third form is like the second one, but it takes a list of finfo
 * on stdin (like a filter does). In all cases, the fpath should be a
 * "spool file" path, that is, a path to a file in the spool directory with
 * the usual convention used by nbsp.
 *
 * If [-f] is not given to specify the location of the nbspd.infifo file,
 * then the default nbsp is used (as set in defaults.h.in).
 *
 * NOTE:  In this mode the Nbsp processor is not invoked and the files
 *        that are saved in the spool directory by the caller of this script,
 *        or by this script itself, are not inserted by the spooldb that
 *        handles the deletion of the spooled files. Therefore the files
 *        that are sent to Nbsp by this script must be deleted from the spool
 *        by the caller of this script, or by the cleanup scheduler.
 *        The latter is set automaticaly by the default hourly-cleanup.conf,
 *        but the files written by this script must be writable by the
 *        nbsp user (noaaport:noaaport) so that they can be deleted by the
 *        default cleanup process.
 *
 *        For more general instructions see the file "nbspinsert.README".
 *        For an example of how to use this script see the "craftinsert" script
 *        and "craftinsert.README".
 */
#include <libgen.h> /* basename */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>	/* flock() */
#include "err.h"
#include "util.h"
#include "fifo.h"

/* defaults */
#define NBSP_INFIFO_FPATH	"/var/run/nbsp/infeed.fifo"

static struct {
  char *seq;
  char *type;
  char *cat;
  char *code;
  char *npchidx;
  char *fname;
  char *fpath;
  char *opt_nbsp_infifo_fpath; /* [-f] */
  int opt_C;            /* check and exit */
  int opt_background;
  /* variables */
  int nbsp_infifo_fd;
} g = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, -1};

static void cleanup(void);
static void check(void);
static int open_nbsp_infifo(void);
static void close_nbsp_infifo(void);
static int process_file(void);

int main(int argc, char **argv){

  char *optstr = "bCf:";
  char *usage = "nbspinsert [-C] [-b] [-f <fifo>] <finfo>";
  int c;
  int status = 0;
  
  set_progname(basename(argv[0]));

  /* defaults */
  g.opt_nbsp_infifo_fpath = NBSP_INFIFO_FPATH;

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'C':
      g.opt_C = 1;
      break;
    case 'f':
      g.opt_nbsp_infifo_fpath = optarg;
      break;
    case 'h':
    default:
      status = 1;
      log_errx(1, "%s\n", usage);
      break;
    }
  }

  if(g.opt_background == 1)
    set_usesyslog();

  if(optind != argc - 7)
    log_errx(1, "%s\n", usage);

  g.seq = argv[optind++];
  g.type = argv[optind++];
  g.cat = argv[optind++];
  g.code = argv[optind++];
  g.npchidx = argv[optind++];
  g.fname = argv[optind++];
  g.fpath = argv[optind++];
  g.fpath = argv[optind++];
  
  atexit(cleanup);
  
  if (g.opt_C == 1) {
    check();
    return(0);
  }

  status = open_nbsp_infifo();
  
  if(status == 0)
    status = process_file();

  close_nbsp_infifo();

  return(status != 0 ? 1 : 0);
}

static void cleanup(void) {

  close_nbsp_infifo();
}

static void check(void){

  fprintf(stdout, "opt_nbsp_infifo_fpath: %s\n", g.opt_nbsp_infifo_fpath);
  fprintf(stdout, "opt_C: %d\n", g.opt_C);
  fprintf(stdout, "opt_background: %d\n", g.opt_background);
}

static int open_nbsp_infifo(void) {

  int fd = -1;
  int status = 0;

  status = check_fifo(g.opt_nbsp_infifo_fpath);

  if(status == -1)
    log_err(0, "Error from stat: %s", g.opt_nbsp_infifo_fpath);
  else if(status != 0) {
    status = 1;
    log_errx(0, "Not a fifo: %s\n", g.opt_nbsp_infifo_fpath); 
  }
  
  if(status != 0)
    return(status);

  /*
   * Open it in blocking mode so that the write() is blocked until nbsp
   * has read enough from the pipe to make space for the write(). Otherwise
   * we will get errors like "Resource tempoarily unvailable" 
   * and loss of packets when the pipe gets full.
   */
  fd = open(g.opt_nbsp_infifo_fpath, O_WRONLY);
  if(fd == -1) {
    log_err(0, "Error from open: %s\n", g.opt_nbsp_infifo_fpath);
    return(-1);
  }

  status = flock(fd, LOCK_EX);
  if(status == -1) {
    log_err(0, "Error from flock: %s\n", g.opt_nbsp_infifo_fpath);

    if(close(fd) == -1)
      log_err(0, "Error from close: %s\n", g.opt_nbsp_infifo_fpath);
    
    return(-1);
  }
  
  g.nbsp_infifo_fd = fd;

  return(0);
}

static void close_nbsp_infifo(void) {

  int status = 0;
  
  if(g.nbsp_infifo_fd == -1)
    return;

  status = flock(g.nbsp_infifo_fd, LOCK_UN);
  if(status == -1)
    log_err(0, "Error unlocking infeed fifo: %s\n", g.opt_nbsp_infifo_fpath);
  
  status = close(g.nbsp_infifo_fd);
  g.nbsp_infifo_fd = -1;

  if(status != 0)
    log_err(0, "Error closing infeed fifo: %s\n", g.opt_nbsp_infifo_fpath);
}

static int process_file(void) {

  int status = 0;

  return(status);
}

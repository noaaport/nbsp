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
 * (See the function validate_input() below for some explanation about
 * the (numerical) first five parameters.)
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
#include <string.h>
#include <sys/file.h>	/* flock() */
#include <stdint.h>
#include "err.h"
#include "util.h"
#include "fifo.h"
#include "nutil.h"

/*
 * The data that is sent to the infeed fifo is the "finfo" string,
 * preceeded by the (four bytes [big endian]) size of the finfo string.
 */

/* defaults */
#define NBSP_INFIFO_FPATH	"/var/run/nbsp/infeed.fifo"
#define FINFO_BUFFER_SIZE 1024 /* see create_finfo() */

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
  char finfo_buffer_s[FINFO_BUFFER_SIZE]; /* static store for the finfo data */
  char *finfo_buffer_d; /* dynamic if the static is not big enough */
  char *finfo_data; /* pointer to the finfo data */
  size_t finfo_data_size; /* finfo + the initial four bytes */
} g = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0,
       -1, "", NULL, NULL, 0};

static void cleanup(void);
static void check(void);
static int validate_input(void);
static int open_nbsp_infifo(void);
static void close_nbsp_infifo(void);
static int process_file(void);
static int create_finfo(void);
static void delete_finfo(void);
static int send_finfo(void);

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
    log_errx(1, "%s %s", "Not arguments.", usage);

  g.seq = argv[optind++];
  g.type = argv[optind++];
  g.cat = argv[optind++];
  g.code = argv[optind++];
  g.npchidx = argv[optind++];
  g.fname = argv[optind++];
  g.fpath = argv[optind++];
    
  if (g.opt_C == 1) {
    check();
    return(0);
  }

  status = validate_input();
  if(status != 0) {
    /* 
     * if status != 0 this is not reached because validate_input()
     * exits the program.
     */
    return(status);
  }

  atexit(cleanup);

  status = open_nbsp_infifo();
  
  if(status == 0)
    status = process_file();

  close_nbsp_infifo();

  return(status != 0 ? 1 : 0);
}

static void cleanup(void) {

  close_nbsp_infifo();
  delete_finfo();
}

static void check(void){

  fprintf(stdout, "opt_nbsp_infifo_fpath: %s\n", g.opt_nbsp_infifo_fpath);
  fprintf(stdout, "opt_C: %d\n", g.opt_C);
  fprintf(stdout, "opt_background: %d\n", g.opt_background);
}

static int validate_input(void) {
  /*
   * The first five parameters in the argument of this program are
   * (see. e.g., src/{packfpc.h,packfp.h})
   *
   * uint32_t seq_number;
   * int psh_product_type;
   * int psh_product_category;
   * int psh_product_code;
   * int np_channel_index;
   *
   * The type, cat, code, index are actually uchar in the sense of their range.
   */
  int status = 0;
  uint32_t u32;
  uint u;

  status = strto_u32(g.seq, &u32);
  if (status != 0) {
    log_errx(1, "%s: %s", "Invalid input: seq", g.seq);
  }

  status = strto_uint(g.type, &u);
  if(status == 0) {
    if(u > 255)
      status = 1;
  }
  if (status != 0) {
    log_errx(1, "%s: %s", "Invalid input: type", g.type);
  }

  status = strto_uint(g.cat, &u);
  if(status == 0) {
    if(u > 255)
      status = 1;
  }
  if (status != 0) {
    log_errx(1, "%s: %s", "Invalid input: cat", g.cat);
  }

  status = strto_uint(g.code, &u);
  if(status == 0) {
    if(u > 255)
      status = 1;
  }
  if (status != 0) {
    log_errx(1, "%s: %s", "Invalid input: code", g.code);
  }

  status = strto_uint(g.npchidx, &u);
  if(status == 0) {
    if(u > 255)
      status = 1;
  }
  if (status != 0) {
    log_errx(1, "%s: %s", "Invalid input: npchidx", g.npchidx);
  }

  return(status);
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
   * we will get errors like "Resource temporarily unavailable" 
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

  status = create_finfo();

  sleep(7);
    
  if(status == 0)
    status = send_finfo();

  delete_finfo();
  
  return(status);
}

static int create_finfo(void) {
  /*
   * The format of the packet sent to the fifo is a file info (finfo)
   * string with the format
   *
   * "%u %d %d %d %d %s %s\n" seq type cat code npchindex fname fpath
   *
   * and this preceeded by four bytes (in big endian order) indicating
   * the size of that string, not including the final '\n'; i.e.,
   * <bytes0-3><finfo>\n.
   *
   * This function joins all the arguments received by the program
   * in one string and builds up the finfo data.
   */
  int status = 0;
  int n;
  char *p = NULL;
  size_t p_size;
  size_t finfo_str_size; /* size of the finfo string without the '\n' */
  size_t finfo_size; /* size required to hold the <bytes0-3><finfo>\n */

  /* First try the static storage */
  p = &g.finfo_buffer_s[4];
  p_size = FINFO_BUFFER_SIZE - 4;  

  /* The terminating \0 will be replaced by the \n at the end */
  n = snprintf(p, p_size, "%s %s %s %s %s %s %s",
	       g.seq, g.type, g.cat, g.code, g.npchidx, g.fname, g.fpath);

  if(n < 0) {
    log_errx(0, "%s", "Error from snprintf.");
    return(1);
  }

  /* We now know */
  finfo_str_size = n;
  finfo_size = n + 1 + 4; /* size of the <bytes0-3><finfo>\n data */

  if((size_t)n < p_size) {
    /* We are done */
    /* Replace the \0 by a cr */    
    p[n] = '\n';

    /* reset p to the start of the storage */
    p = &g.finfo_buffer_s[0];
  } else {
    /* Use dynamic storage */
    g.finfo_buffer_d = malloc(finfo_size);
    if(g.finfo_buffer_d == NULL) {
      log_err(0, "%s", "Error concatenating input parameters.");

      return(1);
    }

    p = &g.finfo_buffer_d[4];
    p_size = finfo_size - 4;

    /* The terminating \0 will be replaced by the \n at the end */
    n = snprintf(p, p_size, "%s %s %s %s %s %s %s",
		 g.seq, g.type, g.cat, g.code, g.npchidx, g.fname, g.fpath);

    if(n < 0) {
      log_errx(0, "%s", "Error from snprintf.");
      status = 1;
    } else if((size_t)n >= p_size) {
      /* This should not happen */
      log_errx(0, "%s", "Error from snprintf - BUG in nbspinsert.");
      status = 1;
    }

    if(status != 0)
      goto End;

    /* Replace the \0 by cr and reset p to the start of the storage */
    p[n] = '\n';
    p = &g.finfo_buffer_d[0];
  }

  if(finfo_str_size > UINT32_MAX) {
    log_errx(0, "%s", "finfo too large.");
    status = 1;
    goto End;
  } else {
    pack_uint32(p, (uint32_t)finfo_str_size, 0);
  }

  g.finfo_data = p;
  g.finfo_data_size = finfo_size;

 End:

  if(status != 0)
    delete_finfo();

  return(status);
}

static void delete_finfo(void) {
  /*
   * If the storage for the finfo was dynamically allocated, free it.
   */
  if(g.finfo_buffer_d != NULL) {
    free(g.finfo_buffer_d);
    g.finfo_buffer_d = NULL;
  }
}

static int send_finfo(void) {

  int status = 0;
  ssize_t nwrite;

  nwrite = write(g.nbsp_infifo_fd, g.finfo_data, g.finfo_data_size);
  if(nwrite == -1) {
    log_err(0, "%s", "Error writing to infeed fifo.");
    status = 1;
  }

  return(status);
}

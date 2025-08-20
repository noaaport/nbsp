/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
/*
 * Usage: nbspmon [-r secs] [-s secs]
 *
 * -r => read timeout secs (default is 60)
 * -s => stats cycle secs (for updating the data rate and file count - 10 secs)
 */
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <curses.h>
#include "nreadn.h"
#include "file.h"
#include "stoi.h"
#include "defaults.h"

/* defaults */
#define READ_TIMEOUT_SECS   60
#define STATS_CYCLE_SECS    10  /* for updating the data rate and file count */

struct {
  char *filterdir;
  char *fname;
  int fd;
  unsigned int count;
  off_t filesize;
  time_t lasttime;
  int stats_cycle_secs;
  int read_timeout_secs;
  unsigned int rate;
  int f_quit;
  char *p;	/* transmission data */
  size_t psize;	/* allocated size of p */
} grmon = {NBSP_FILTER_DEVDIR, NULL, -1,
  0, 0, 0,
  STATS_CYCLE_SECS, READ_TIMEOUT_SECS, 0, 0,
  NULL, 0};

static char *usage = "nbspmon [-r secs] [-s secs]";

static int parse_args(int argc, char **argv);
static int validate_args(void);
static int rmon_init(char *progname);
static int rmon_loop(void);
static int rmon_run(void);
static void rmon_cleanup(void);
static void init_curses(void);
static void clean_curses(void);

/*
 * int main(int argc __attribute__((unused)), char **argv){
 */

int main(int argc, char **argv){

  int status = 0;

  status = parse_args(argc, argv);
  if(status == 0)
    status = validate_args();

  if(status != 0)
    exit(EXIT_FAILURE);

  atexit(rmon_cleanup);
  status = rmon_init(argv[0]);

  if(status == 0)
    init_curses();

  if(status == 0)
    status = rmon_run();

  return(status);
}

static int parse_args(int argc, char ** argv){

  char *optstr = "r:s:";
  int status = 0;
  int c;

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'r':
      status = strto_int(optarg, &grmon.read_timeout_secs);
      if(status == 1){
	errx(1, "%s", "Invalid argument to [-r] option.");
      }
      break;
    case 's':
      status = strto_int(optarg, &grmon.stats_cycle_secs);
      if(status == 1){
	errx(1, "%s", "Invalid argument to [-s] option.");
      }
      break;
    default:
      status = 1;
      errx(1, "%s", usage);
      break;
    }
  }

  return(status);
}

static int validate_args(void){

  int status = 0;

  if(grmon.read_timeout_secs <= 0){
    status = 1;
    errx(1, "Illegal value of [-r] option.");
  }

  if(grmon.stats_cycle_secs <= 0){
    status = 1;
    errx(1, "Illegal value of [-s] option.");
  }

  return(status);
}

static int rmon_init(char *progname){

  int status = 0;
  int fname_size;
  int n;

  fname_size = strlen(grmon.filterdir) + strlen(basename(progname)) + 5 + 1;
  grmon.fname = malloc(fname_size + 1);
  if(grmon.fname == NULL){
    err(1, "Cannot create fifo.");
    return(-1);
  }

  n = snprintf(grmon.fname, fname_size + 1, "%s/%s.fifo", grmon.filterdir,
	       basename(progname));
  assert(n == fname_size);
  
  unlink(grmon.fname);
  status  = mkfifo(grmon.fname, 0644);

  if(status == 0){
    grmon.fd = open(grmon.fname, O_RDONLY | O_NONBLOCK, 0);
  }

  if(grmon.fd == -1){
    err(1, "Cannot open %s", grmon.fname);
    return(-1);
  }

  grmon.lasttime = time(NULL);
  
  return(status);
}

static int rmon_run(void){

  int status = 0;

  while((status == 0) && (grmon.f_quit == 0)){
    move(0,0);
    status = rmon_loop();
    refresh();
  }

  return(status);
}

static int rmon_loop(void){

  int n;
  char header[12];
  int HSIZE = 12;
  char *p = NULL;
  size_t size;
  off_t fsize;	/* file size */
  char *fpath;
  char *fname;
  /* int id; */
  time_t now;
  struct tm *tmptr;
  int fd = grmon.fd;
  int status = 0;

  n = readn_fifo(fd, header, HSIZE, grmon.read_timeout_secs);

  if(n == 0){
    printw("Timed out before anything could be read (poll timed out)");
    grmon.f_quit = 1;
    return(0);
  }

  if(n == -1){
    status = -1;
  }else if(n != HSIZE){
    status = 1;
  }

  if(status != 0)
    goto end;

  /*
   * id = (header[0] << 24) + (header[1] << 16) + (header[2] << 8) + header[3];
   */

  size = (header[4] << 24) + (header[5] << 16) 
    + (header[6] << 8) + header[7];

  /*
   * The transmission is the full path name of the file, preceeded
   * by the product data codes (which include the fname).
   */
  if(size > grmon.psize){
    p = realloc(grmon.p, size);
    if(p == NULL){
      status = -1;
      goto end;
    } else {
      grmon.p = p;
      grmon.psize = size;
    }    
  } else
    p = grmon.p;

  n = readn_fifo(fd, p, size, 1);

  if(n == 0){
    printw("Timed out before anything could be read (poll timed out)");
    grmon.f_quit = 1;
    return(0);
  }

  if(n == -1){
    status = -1;
  }else if((size_t)n != size){
    status = 1;
  }

  if(status != 0)
    goto end;

  /*
   * p[0-7] contain the product specific data, defined in packfp.c.
   * We are not using that here, so we extract only the fname and fpath.
   */

  fname = &p[8];
  fpath = &p[8 + FNAME_SIZE + 1];
  ++grmon.count; 
  
  status = get_file_size(fpath, &fsize);
  if(status == 0){
    grmon.filesize += fsize;
  }

  now = time(NULL);
  tmptr = localtime(&now);
  if(now > grmon.lasttime + grmon.stats_cycle_secs){
    grmon.rate = grmon.filesize/(grmon.stats_cycle_secs * 1000);
    grmon.filesize = 0;
    grmon.count = 0;
    grmon.lasttime = now;
  }

  printw("Processing %s\n", fname);
  printw(" Processed %u\n", grmon.count);
  printw(" Data Rate %u kbs\n", grmon.rate);
  printw("      Time %02d:%02d:%02d\n", 
	 tmptr->tm_hour, tmptr->tm_min, tmptr->tm_sec);

 end:

  if(status == -1)
    printw("Error reading from fifo. %s", strerror(errno));
  else if(status != 0)
    printw("Disconnection detected while reading.");

  return(status);
}

static void rmon_cleanup(void){
  
  if(grmon.fd != -1)
    close(grmon.fd);

  if(grmon.fname != NULL){
    unlink(grmon.fname);
    free(grmon.fname);
  }
}

static void init_curses(void){

  initscr(); cbreak(); noecho(); nonl();
  intrflush(stdscr, FALSE); 
  keypad(stdscr, TRUE);

  atexit(clean_curses);
}

static void clean_curses(void){

  (void)endwin();
}

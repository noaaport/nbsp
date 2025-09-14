/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
/*
 * Usage: rnbspmon [-r secs] [-s secs] <server>
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
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <curses.h>
#include <netdb.h>
#include "libconnth/libconn.h"
#include "nreadn.h"
#include "stoi.h"
#include "const.h"

#define PORT		"2210"
#define NBS2STR		"NBS2"

/* defaults */
#define READ_TIMEOUT_SECS   60
#define STATS_CYCLE_SECS    10  /* for updating the data rate and file count */

struct {
  char *server;
  int fd;
  unsigned int count;
  time_t lasttime;
  int stats_cycle_secs;
  int read_timeout_secs;
  int f_quit;
  char *p;	/* transmission data */
  size_t psize;	/* allocated size of p */
} grmon = {NULL, -1, 0, 0, STATS_CYCLE_SECS, READ_TIMEOUT_SECS, 0, NULL, 0};

static char *usage = "rnbspmon [-r secs] [-s secs] <server>";

static int parse_args(int argc, char **argv);
static int validate_args(void);
static int rmon_init(void);
static int rmon_loop(void);
static int rmon_run(void);
static void rmon_cleanup(void);
static void init_curses(void);
static void clean_curses(void);
static void signal_init(void);
static void signal_handler(int sig);

int main(int argc, char **argv){

  int status = 0;

  status = parse_args(argc, argv);
  if(status == 0)
    status = validate_args();

  if(status != 0)
    exit(EXIT_FAILURE);

  atexit(rmon_cleanup);
  
  signal_init();
  status = rmon_init();

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

  if(optind != argc - 1){
    status = 1;
    errx(1, "%s", usage);
  }

  grmon.server = argv[optind++];

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

static int rmon_init(void) {

  int fd = -1;
  int gai_code;
  int n;
  char *port = PORT;
  char *nbs2str = NBS2STR;
  int status = 0;

  fd = tcp_client_open_conn(grmon.server, port, -1, -1, &gai_code);
  if(fd == -1){
    if(gai_code != 0)
      errx(1, "tcp_client. %s", gai_strerror(gai_code));
    else
      err(1, "tcp_client_open_conn()");

    return(-1);
  }

  /* wait for 1 sec and don't retry */
  n = writem(fd, nbs2str, strlen(nbs2str), 1000, 0);
  if(n < 0){
    close(fd);
    err(1, "writem()");

    return(-1);
  }

  grmon.fd = fd;
  grmon.lasttime = time(NULL);
  
  return(status);
}

static int rmon_run(void) {

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
  char *fname;
  /* char *fpath; */
  /* int id; */
  time_t now;
  struct tm *tmptr;
  int fd = grmon.fd;
  int status = 0;

  n = readn(fd, header, HSIZE, grmon.read_timeout_secs, 0);
   
  if(n == -2){
    /* timed out before anything could be read (poll timed out) */
    status = -2;
  }else if(n < 0){
    /* real error (-1) */
    status = -1;
  }else if(n != HSIZE){
    /*
     * number of characters read (includes partial read if poll timed out
     * or disconnection ocurred while reading, and 0 if disconnection
     * was detected before reading anything)
     */
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
   * by the product specific data codes.
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

  n = readn(fd, p, size, grmon.read_timeout_secs, 0);

  if(n == -2){
    status = -2;
  }else if(n < 0){
    status = -1;
  }else if((size_t)n != size){
    status = 1;
  }

  if(status != 0)
    goto end;

  /*
   * p[0-7] contain the product specific data, defined in nsbp.c.
   * We are not using that here, so we extract only the fname and fpath.
   */
  fname = &p[8];
  /* fpath = &p[8 + FNAME_SIZE + 1]; */
  ++grmon.count; 
  
  now = time(NULL);
  tmptr = localtime(&now);
  if(now > grmon.lasttime + grmon.stats_cycle_secs){
    grmon.count = 0;
    grmon.lasttime = now;
  }

  printw("Receiving %s\n", fname);
  printw(" Received %u\n", grmon.count);
  printw("     Time %02d:%02d:%02d\n", 
	 tmptr->tm_hour, tmptr->tm_min, tmptr->tm_sec);

 end:

  if(status != 0) {
    if(status == -2)
      printw("Timed out before anything could be read (poll timed out)");
    else if(status == -1)    
      printw("Error reading from socket. %s", strerror(errno));
    else
      printw("Disconnection detected before or while reading.");
  }
  
  return(status);
}

static void rmon_cleanup(void){

  if(grmon.fd != -1)
    close(grmon.fd);

  if(grmon.p != NULL)
    free(grmon.p);
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

static void signal_init(void) {

	struct sigaction sa;

	sa.sa_handler = signal_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask); 
	
	sigaction(SIGINT, &sa, NULL);
}

static void signal_handler(int sig) {

	(void)sig;
	grmon.f_quit = 1;
}

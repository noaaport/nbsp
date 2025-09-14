/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
/*
 * Usage: rnbspmon1 [-r secs] [-s secs] <server>
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
#include "nbs1.h"

#define PORT		"2210"
#define NBS1STR		"NBS1"

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
} grmon = {NULL, -1, 0, 0, STATS_CYCLE_SECS, READ_TIMEOUT_SECS, 0};

static char *usage = "rnbspmon1 [-r secs] [-s secs] <server>";

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
  char *nbs1str = NBS1STR;
  int status = 0;

  fd = tcp_client_open_conn(grmon.server, port, -1, -1, &gai_code);
  if(fd == -1){
    if(gai_code != 0)
      errx(1, "tcp_client_open_conn. %s", gai_strerror(gai_code));
    else
      err(1, "tcp_client_open_conn()");

    return(-1);
  }

    /* wait for 1 sec and don't retry */
  n = writem(fd, nbs1str, strlen(nbs1str), 1000, 0);
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

  struct nbs1_packet_st nbs;
  time_t now;
  struct tm *tmptr;
  int fd = grmon.fd;
  int status = 0;

  /* no retry */
  status = recv_nbs_packet(fd, &nbs, grmon.read_timeout_secs, 0);

  if(status != 0) {
    if(status == -1)
      printw("Error reading from socket. %s", strerror(errno));
    else if(status == -2)
      printw("Timed out (poll) before reading anything.");
    else if(status == -3)
      printw("Disconnected (eof) before reading anything.");
    else if(status == 1)
      printw("Short read (time out or disconnect) while reading.");
    else if(status == 2) {
      printw("Checksum error (corrupt packet) or incorrect type of packet.");
      status = 0;
    }

    return(status);
  }

  ++grmon.count; 
  
  now = time(NULL);
  tmptr = localtime(&now);
  if(now > grmon.lasttime + grmon.stats_cycle_secs){
    grmon.count = 0;
    grmon.lasttime = now;
  }

  printw("Receiving %s : %d\n", nbs.fbasename, nbs.num_blocks);
  printw(" Received %d : %u\n", nbs.block_number, (unsigned int)nbs.block_size);
  printw("     Time %02d:%02d:%02d\n", 
	 tmptr->tm_hour, tmptr->tm_min, tmptr->tm_sec);

  return(status);
}

void rmon_cleanup(void){

	if(grmon.fd != -1)
		close(grmon.fd);
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

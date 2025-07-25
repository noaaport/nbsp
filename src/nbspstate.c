#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <curses.h>
#include "nreadn.h"
#include "defaults.h"

/* globals */
struct {
  char *fname;		/* name of queue state fifo */
  uint32_t *buffer;	/* working buffer */
  int buffer_size;
  int fd;		/* working fd of fifo */
  int f_quit;					
} gqstate = {NBSP_QSTATEFIFO, NULL, 0, -1, 0};

static int qstate_init(void);
static int qstate_loop(void);
static int qstate_run(void);
static void qstate_cleanup(void);
static void init_curses(void);
static void clean_curses(void);

int main(void){

  int status = 0;

  status = qstate_init();
  atexit(qstate_cleanup);

  if(status == 0)
    init_curses();

  while((gqstate.f_quit == 0) && (status == 0)){
    status = qstate_run();
    if(status != 0){
      qstate_cleanup();
      status = qstate_init();
    }
  }

  return(status);
}

static int qstate_init(void){

  int status = 0;

  gqstate.fd = open(gqstate.fname, O_RDONLY | O_NONBLOCK, 0);

  if(gqstate.fd == -1){
    status = -1;
    err(1, "Cannot open %s.", gqstate.fname);
  }

  return(status);
}

static int qstate_run(void){

  int status = 0;

  while((gqstate.f_quit == 0) && (status == 0)){
    move(0,0);
    status = qstate_loop();
    refresh();
  }

  return(status);
}

static int qstate_loop(void){

  int n;
  int status = 0;
  int size;
  uint32_t numchannels;
  int nchannels_serverq;
  int i;
  int start_n, start_nmax;
  time_t now;
  struct tm *tmptr;
  int fd = gqstate.fd;
  void *buffer;

  size = sizeof(uint32_t);
  n = readn_fifo(fd, &numchannels, size, 30);

  if(n == 0){
    gqstate.f_quit = 1;
    return(0);
  }

  if(n == -1){
    status = -1;
  }else if(n != size){
    status = 1;
  }

  if(status != 0)
    goto end;

  nchannels_serverq = (int)numchannels;

  size = sizeof(uint32_t) * 2 * (nchannels_serverq + 1);
  if(gqstate.buffer == NULL){
    gqstate.buffer = (uint32_t*)malloc(size);
    if(gqstate.buffer != NULL)
      gqstate.buffer_size = size;
  }else if(gqstate.buffer_size != size){
    buffer = realloc(gqstate.buffer, size);
    if(buffer == NULL){
      free(gqstate.buffer);
      gqstate.buffer = NULL;
      gqstate.buffer_size = 0;
    }else{
      gqstate.buffer = (uint32_t*)buffer;
      gqstate.buffer_size = size;
    }
  }

  if(gqstate.buffer == NULL){
    status = -1;
    goto end;
  }

  n = readn_fifo(fd, gqstate.buffer, size, 2);

  if(n == 0){
    gqstate.f_quit = 1;
    return(0);		/* disconnection */
  }

  if(n == -1){
    status = -1;
  }else if(n != size){
    status = 1;
  }

  if(status != 0)
    goto end;

  start_n = 0;
  start_nmax = nchannels_serverq + 1;

  printw("pctl: n = %u, nmax = %u\n",
	 gqstate.buffer[start_n++], gqstate.buffer[start_nmax++]);

  for(i = 0; i < nchannels_serverq; ++i){
    printw("queue[%d]: n = %u, nmax = %u\n", i,
	   gqstate.buffer[start_n++], gqstate.buffer[start_nmax++]);
  }

  now = time(NULL);
  tmptr = localtime(&now);
  printw("\nTime %02d:%02d:%02d\n",
	 tmptr->tm_hour, tmptr->tm_min, tmptr->tm_sec);

 end:

  if(status == -1)
    printw("Error reading from fifo or data file. %s", strerror(errno));
  else if(status != 0)
    printw("Timed out reading from fifo.");

  return(status);
}

static void qstate_cleanup(void){

  if(gqstate.fd != -1){
    close(gqstate.fd);
    gqstate.fd = -1;
  }

  if(gqstate.buffer != NULL){
    free(gqstate.buffer);
    gqstate.buffer = NULL;
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

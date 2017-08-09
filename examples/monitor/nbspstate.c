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
#include "readn.h"

static char *fname = "/var/run/nbspd.fifo";
static int fd = -1;
static int f_quit = 0;

static int loop(void);
static void cleanup(void);
static void init_curses(void);

int main(int argc, char **argv){

  int status = 0;

  atexit(cleanup);

  fd = open(fname, O_RDONLY | O_NONBLOCK, 0);
  if(fd == -1)
    err(1, "open()");

  init_curses();

  while(f_quit == 0){
    move(0,0);
    status = loop();
    refresh();
  }

  return(status);
}

int loop(void){

  int n;
  int status = 0;
  int size;
  int *buffer = NULL;
  int numchannels;
  int i;

  size = sizeof(int);
  n = readm(fd, &numchannels, size, 3000);
  if(n == -2){
    if(read(fd, buffer, size) == 0)
      n = 0;
  }

  if(n == -1){
    status = -1;
  }else if(n == 0){
    f_quit = 1;
    return(1);
  }else if(n != size){
    status = 1;
  }

  if(n != size)
    goto end;

  size = sizeof(int) * 2 * (numchannels + 1);
  buffer = malloc(size);
  if(buffer == NULL){
    status = -1;
    goto end;
  }

  /*
   * n == 0 (eof) we return 0 because we will
   * come back to read in the next loop. For a partial read we return error.
   */
  n = readm(fd, buffer, size, 1000);
  if(n == -2){
    if(read(fd, buffer, size) == 0)
      n = 0;
  }

  if(n == -1){
    status = -1;
  }else if(n == 0){
    f_quit = 1;
    return(1);		/* disconnection */
  }else if(n != size){
    status = 1;
  }

  if(n != size)
    goto end;

  printw("pctl: n = %d, nmax = %d\n", buffer[0], buffer[numchannels + 1]);
  for(i = 0; i <= numchannels - 1; ++i){
    printw("queue[%d]: n = %d, nmax = %d\n", i, buffer[i + 1], 
	   buffer[numchannels + 2 + i]);
  }

 end:

  if(status == -1)
    printw("Error reading from fifo or data file. %s", strerror(errno));
  else if(status > 0)
    printw("Timed out reading from fifo.");

  if(buffer != NULL)
    free(buffer);

  return(status);
}

void cleanup(void){

  if(fd != -1)
    close(fd);
}

static void init_curses(void){

  initscr(); cbreak(); noecho(); nonl();
  intrflush(stdscr, FALSE); 
  keypad(stdscr, TRUE);

}

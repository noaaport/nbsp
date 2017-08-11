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
#include "file.h"

#define DIR		"/var/noaaport/nbsp/dev"

static char *fname = NULL;
static unsigned int gcount = 0;
static  off_t gfilesize = 0;
static time_t gtime;
static int gcycle_secs = 10;
static unsigned int grate = 0;
static int f_quit = 0;

static int loop(int fd);
static void cleanup(void);
static void init_curses(void);

int main(int argc, char **argv){

  int fd = -1;
  int status = 0;
  int fname_size;
  int n;

  fname_size = strlen(DIR) + strlen(basename(argv[0])) + 5 + 1;

  fname = malloc(fname_size + 1);
  if(fname == NULL)
    err(1, "Cannot create fifo.");

  n = snprintf(fname, fname_size + 1, "%s/%s.fifo", DIR, basename(argv[0]));
  assert(n == fname_size);

  unlink(fname);
  status  = mkfifo(fname, 0644);

  atexit(cleanup);

  /*
   * Thus, like any other filter, must be opened blocking or otherwise
   * we have to wait some time to give time to the server to install
   * and open the filter.
   */
  if(status == 0){
    fd = open(fname, O_RDONLY | O_NONBLOCK, 0);
  }

  if(fd == -1)
    err(1, "open()");

  gtime = time(NULL);
  init_curses();

  while(f_quit == 0){
    move(0,0);
    status = loop(fd);
    refresh();
  }

  close(fd);

  return(status);
}

int loop(int fd){

  int n;
  char header[12];
  char *p = NULL;
  char *fpath;
  int id;
  off_t size;
  int status = 0;

  size = 12;
  n = readm(fd, header, size, 5000);

  if(n == -2){
    status = -2;
  }else if(n < 0){
    status = -1;
  }else if(n != size){
    status = 1;
  }
  
  if(status != 0)
    goto end;

  id = (header[0] << 24) + (header[1] << 16) + (header[2] << 8) + header[3];
  size = (header[4] << 24) + (header[5] << 16) 
    + (header[6] << 8) + header[7];

  /*
   * The transmission is the full path name of the file, preceeded
   * by the product data codes.
   */
  p = malloc(size);
  if(p == NULL){
    status = -1;
    goto end;
  }

  n = readm(fd, p, size, 1000);

  if(n == -2){
    status = -2;
  }else if(n == -1){
    status = -1;
  }else if(n < size){
    status = 1;
  }

  if(status != 0)
    goto end;

  /*
   * p[0-7] contain the product specific data, defined in nsbp.c. That
   * will be transmitted in the NBS ptotocol but I don't know how to
   * use it for emwin. So, for emwin, we extract only the path.
   */

  /*  p[size] = '\0'; */
  fpath = &p[8];
  ++gcount; 
  
  status = get_file_size(fpath, &size);
  if(status == 0){
    gfilesize += size;
  }

  if(time(NULL) > gtime + gcycle_secs){
    grate = gfilesize/(gcycle_secs * 1000);
    gfilesize = 0;
    gcount = 0;
    gtime = time(NULL);
  }

  printw("Receiving %s\n", fpath);
  printw(" Received %u\n", gcount);
  printw("Data Rate %u kbs\n", grate);

 end:

  if(status == -1)
    printw("Error reading from fifo or data file. %s", strerror(errno));
  else if(status != 0)
    printw("Timed out reading from fifo.");

  if(p != NULL)
    free(p);

  return(status);
}

void cleanup(void){
  
  if(fname != NULL){
    unlink(fname);
    free(fname);
  }
  
}

static void init_curses(void){

  initscr(); cbreak(); noecho(); nonl();
  intrflush(stdscr, FALSE); 
  keypad(stdscr, TRUE);

}

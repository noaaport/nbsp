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
#include <netdb.h>
#include "libconnth/libconn.h"
#include "readn.h"
#include "const.h"

#define PORT		"1000"
#define HOST		"caribe"
#define NBS2STR		"NBS2"

static char *fname = NULL;
static unsigned int gcount = 0;
static time_t gtime;
static int gcycle_secs = 10;
static int f_quit = 0;

static int loop(int fd);
static void cleanup(void);
static void init_curses(void);

int main(void){

  int fd = -1;
  int status = 0;
  int gai_code;
  int n;
  char *host = HOST;
  char *port = PORT;
  char *nbs2_str = NBS2STR;

  fd = tcp_client_open_conn(host, port, -1, -1, &gai_code);
  if(fd == -1){
    if(gai_code != 0)
      errx(1, "tcp_client. %s", gai_strerror(gai_code));
    else
      err(1, "open()");
  }

  atexit(cleanup);

  n = writem(fd, nbs2_str, strlen(nbs2_str), 1000, 0);
  if(n < 0){
    close(fd);
    fd = -1;
    err(1, "writem()");
  }

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
  char *fname;
  int id;
  off_t size;
  time_t now;
  struct tm *tmptr;

  int status = 0;

  size = 12;
  n = readm(fd, header, size, 5000, 0);

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

  n = readm(fd, p, size, 1000, 0);

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
   * p[0-7] contain the product specific data, defined in nsbp.c.
   * We are not using that here, so we extract only the fname and fpath.
   */
  fname = &p[8];
  fpath = &p[8 + FNAME_SIZE + 1];
  ++gcount; 
  
  now = time(NULL);
  tmptr = localtime(&now);
  if(now> gtime + gcycle_secs){
    gcount = 0;
    gtime = now;;
  }

  printw("Receiving %s\n", fname);
  printw(" Received %u\n", gcount);
  printw("     Time %02d:%02d:%02d\n", 
	 tmptr->tm_hour, tmptr->tm_min, tmptr->tm_sec);

 end:

  if(status != 0)
    printw("Error reading from socket. %s", strerror(errno));

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

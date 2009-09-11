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
#include "nbs1.h"

#define PORT		"1000"
#define HOST		"diablo"
#define NBS_STR		"NBS1"

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
  char *nbs_str = NBS_STR;

  fd = tcp_client_open_conn(host, port, -1, -1, &gai_code);
  if(fd == -1){
    if(gai_code != 0)
      errx(1, "tcp_client. %s", gai_strerror(gai_code));
    else
      err(1, "open()");
  }
  
  atexit(cleanup);

  n = writem(fd, nbs_str, strlen(nbs_str), 1000, 0);
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

  struct nbs1_packet_st nbs;
  time_t now;
  struct tm *tmptr;
  int status = 0;

  status = recv_nbs_packet(fd, &nbs, 5, 2);

  /*
  if(n == -2){
    status = -2;
  }else if(n < 0){
    status = -1;
  }else if(n != size){
    status = 1;
  }
  */
  
  if(status != 0)
    goto end;

  ++gcount; 
  
  now = time(NULL);
  tmptr = localtime(&now);
  if(now > gtime + gcycle_secs){
    gcount = 0;
    gtime = now;
  }

  printw("Receiving %s : %d\n", nbs.fbasename, nbs.num_blocks);
  printw(" Received %d : %u\n", nbs.block_number, (unsigned int)nbs.block_size);
  printw("     Time %02d:%02d:%02d\n", 
	 tmptr->tm_hour, tmptr->tm_min, tmptr->tm_sec);

 end:

  if(status != 0)
    printw("Error reading from socket. %s", strerror(errno));

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

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
#include "defaults.h"

#define READ_TIMEOUT_SECS   60
#define STATS_CYCLE_SECS    10  /* for updating the data rate and file count */

struct {
  char *filterdir;
  char *fname;
  int fd;
  unsigned int count;
  off_t filesize;
  time_t lasttime;
  int cycle_secs;
  unsigned int rate;
  int f_quit;
} grmon = {NBSP_FILTER_DEVDIR, NULL, -1, 0, 0, 0, STATS_CYCLE_SECS, 0, 0};

static int rmon_init(char *progname);
static int rmon_loop(void);
static int rmon_run(void);
static void rmon_cleanup(void);
static void init_curses(void);
static void clean_curses(void);

int main(int argc __attribute__((unused)), char **argv){

  int status = 0;

  status = rmon_init(argv[0]);

  if(status == 0)
    init_curses();

  if(status == 0)
    status = rmon_run();

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

  atexit(rmon_cleanup);

  if(status == 0){
    grmon.fd = open(grmon.fname, O_RDONLY | O_NONBLOCK, 0);
  }

  if(grmon.fd == -1){
    err(1, "Cannot open %s", grmon.fname);
    return(-1);
  }

  grmon.lasttime = time(NULL);
  init_curses();
  
  return(status);
}

static int rmon_run(void){

  int status = 0;

  while(grmon.f_quit == 0){
    move(0,0);
    status = rmon_loop();
    refresh();
  }

  return(status);
}

static int rmon_loop(void){

  int n;
  char header[12];
  char *p = NULL;
  char *fpath;
  char *fname;
  /* int id; */
  off_t size;
  time_t now;
  struct tm *tmptr;
  int fd = grmon.fd;

  int status = 0;

  size = 12;
  n = readn_fifo(fd, header, size, READ_TIMEOUT_SECS);

  if(n == 0){
    grmon.f_quit = 1;
    return(0);
  }

  if(n == -1){
    status = -1;
  }else if(n != size){
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
  p = malloc(size);
  if(p == NULL){
    status = -1;
    goto end;
  }

  n = readn_fifo(fd, p, size, 1);

  if(n == 0){
    grmon.f_quit = 1;
    return(0);
  }

  if(n == -1){
    status = -1;
  }else if(n != size){
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
  
  status = get_file_size(fpath, &size);
  if(status == 0){
    grmon.filesize += size;
  }

  now = time(NULL);
  tmptr = localtime(&now);
  if(now> grmon.lasttime + grmon.cycle_secs){
    grmon.rate = grmon.filesize/(grmon.cycle_secs * 1000);
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
    printw("Error reading from fifo or data file. %s", strerror(errno));
  else if(status != 0)
    printw("Timed out reading from fifo.");

  if(p != NULL)
    free(p);

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

/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>		/* MAXPATHLEN */
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <libgen.h>
#include "err.h"
#include "util.h"

/*
 * This program calls nbspnids, which in turn calls gpmap_gif. See nbspnids.c
 * for a NOTE regarding the input file.
 */

#define NBSPNIDS_BIN		"nbspnids"
#define TIMEOUT_S		90

struct {
  char *opt_input_fname;
  char *opt_output_fname;
  char *opt_wd;			/* working directory */
  char *opt_nbspnids_bin;
  int opt_background;
  int opt_wait_secs;
  char *opt_gpmap_gif_bin;
  char *opt_mapfil;
  char *opt_map;
  char *opt_size;
  char *opt_imcbar;
} g = {NULL, NULL, NULL, NBSPNIDS_BIN, 0, TIMEOUT_S, 
       NULL, NULL, NULL, NULL, NULL};

static int gsigchld = 0;
static int gsigalrm = 0;
static void signal_handler(int sig);
static void signal_init(void);
static void wait_child(int wait_secs);

static int make_path(char **fname);
static int process_file(char *in_file);
static int execvp_wait(int wait_secs, char *file, char **argv);

int main(int argc, char **argv){

  /*
   * The options fimsoq are passed to nbspnids.
   */
  int status = 0;
  int c;
  char *optstr = "hbd:f:i:m:o:p:q:s:vw:";
  char *usage = "nbsprad [-h] [-b] [-d wd] [-f mapfil] [-i imcbar] [-m map] "
    "[-o name] [-p nbspnids_path] [-q gpmap_gif_bin] [-s size] "
    "[-v] [-w secs] file ";

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'd':
      g.opt_wd = optarg;
      break;
    case 'f':
      g.opt_mapfil = optarg;
      break;
    case 'i':
      g.opt_imcbar = optarg;
      break;
    case 'm':
      g.opt_map = optarg;
      break;
    case 'o':
      g.opt_output_fname = optarg;
      break;
    case 'p':
      g.opt_nbspnids_bin = optarg;
      break;
    case 'q':
      g.opt_gpmap_gif_bin = optarg;
      break;
    case 's':
      g.opt_size = optarg;
      break;
    case 'w':
      if(strto_int(optarg, &g.opt_wait_secs) != 0)
	log_errx(1, "[-w] needs an integer argument");

      break;
    case 'h':
    default:
      log_info(usage);
      exit(0);
      break;
    }
  }

  if(optind != argc - 1)
    log_errx(1, "Needs one argument.");

  if(g.opt_background == 1)
    set_usesyslog();

  g.opt_input_fname = argv[optind];

  if(g.opt_wd != NULL){
    status = make_path(&g.opt_input_fname);
    if(status == 0)
      make_path(&g.opt_output_fname);

    if(status == 0){
      status = chdir(g.opt_wd);
      if(status != 0)
	log_err(1, "Cannot change directory to %s.", g.opt_wd);
    }
  }

  if(status == 0)
      status = process_file(g.opt_input_fname);

  return(status != 0 ? 1 : 0);
}

static int process_file(char *inputfile){

  int status = 0;
  char *argv[16];
  int argcN = 16;
  int argc = 0;

  argv[argc++] = g.opt_nbspnids_bin;

  if(g.opt_background == 1)
    argv[argc++] = "-b";

  if(g.opt_output_fname != NULL){
    argv[argc++] = "-o";
    argv[argc++] = g.opt_output_fname;
  }

  if(g.opt_gpmap_gif_bin != NULL){
    argv[argc++] = "-q";
    argv[argc++] = g.opt_gpmap_gif_bin;
  }

  if(g.opt_mapfil != NULL){
    argv[argc++] = "-f";
    argv[argc++] = g.opt_mapfil;
  }

  if(g.opt_imcbar != NULL){
    argv[argc++] = "-i";
    argv[argc++] = g.opt_imcbar;
  }

  if(g.opt_map != NULL){
    argv[argc++] = "-m";
    argv[argc++] = g.opt_map;
  }

  if(g.opt_size != NULL){
    argv[argc++] = "-s";
    argv[argc++] = g.opt_size;
  }

  argv[argc++] = inputfile;
  argv[argc++] = NULL;

  assert(argc <= argcN);

  if(g.opt_wait_secs > 0)
    status = execvp_wait(g.opt_wait_secs, g.opt_nbspnids_bin, argv);
  else
    status = execvp(g.opt_nbspnids_bin, argv);

  if(status != 0)
    log_err(1, "Cannot exec %s", g.opt_nbspnids_bin);

  return(status);
}

static int make_path(char **fname){
  /*
   * Convert fname to a full path.
   */
  char cwd[MAXPATHLEN];
  char *fpath;
  int fpath_size;

  /*
   * Only if the name has been specified and for relative paths. 
   */
  if((*fname == NULL) || (*fname[0] == '/'))
    return(0);

  if(getcwd(cwd, MAXPATHLEN) == NULL)
    log_err(1, "Cannot get current directory.");
  
  fpath_size = strlen(cwd) + strlen(*fname) + 2;
  fpath = malloc(fpath_size);
  if(fpath == NULL){
    log_err(1, "Cannot create path for %s", *fname);
    return(-1);
  }

  snprintf(fpath, fpath_size, "%s/%s/", cwd, *fname);
  *fname = fpath;

  return(0);
}

static int execvp_wait(int wait_secs, char *file, char **argv){

  pid_t pid;
  int status = 0;
  int wait_status = 0;

  /*
   * This sets the signals handlers and blocks alarm and child signals.
   */
  signal_init();

  pid = fork();
  if(pid == -1)
    log_err(1, "Cannot exec %s", file);
  else if(pid == 0){
    status = execvp(file, argv);
    if(status != 0)
      log_err(1, "Cannot exec %s", file);
  }

  /* 
   * The rest is the parent.
   * wait_child() waits for either a child signal or a timeout alarm.
   */
  wait_child(wait_secs);
  status = waitpid(pid, &wait_status, WNOHANG);

  if(status == -1)
    log_err(1, "Error waiting for child.");
  else if(status == 0){
    /*
     * Must kill it since it exceeded the time limit
     */
    kill(pid, SIGKILL);
    waitpid(pid, &wait_status, WNOHANG);
    log_errx(1, "%s exceed time limit. Killed.", file);
  }

  return(0);
}

static void signal_init(void){

  sigset_t newmask;
  int status = 0;

  if(signal(SIGCHLD, signal_handler) == SIG_ERR)
    status = -1;

  if(status == 0){
    if(signal(SIGALRM, signal_handler) == SIG_ERR)
      status = -1;
  }

  /*
   * Block alarm and child signals.
   */ 
  if(status == 0){
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGALRM);
    sigaddset(&newmask, SIGCHLD);  
    status = sigprocmask(SIG_BLOCK, &newmask, NULL);
  }

  if(status != 0)
    log_err(1, "Cannot initialize signal handler.");
}

static void signal_handler(int sig){

  switch(sig){
  case SIGCHLD:
    gsigchld = 1;
    /* log_info("Child terminated."); */
    break;
  case SIGALRM:
    gsigalrm = 1;
    log_info("Received timeout alarm signal.");
    break;
  default:
    break;
  }
}

static void wait_child(int wait_secs){

  sigset_t zeromask;

  sigemptyset(&zeromask);
  alarm(wait_secs);
  sigsuspend(&zeromask);
}

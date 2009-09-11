/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

/*
 * This the forking version of tclhttpd.
 */
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "err.h"
#include "file.h"
#include "tclhttpd.h"

#define TCLHTTPD_CLOSE_SLEEP		1
#define TCLHTTPD_CLOSE_RETRY		10

static int set_tclhttpd_script(struct tclhttpd_st *hp, char *script);
static int set_tclhttpd_fifo(struct tclhttpd_st *hp, char *fifo);
static int tclhttpd_create_child(struct tclhttpd_st *hp);

struct tclhttpd_st *tclhttpd_open(char *script, char *fifo){

  struct tclhttpd_st *hp;
  int status = 0;

  assert(script != NULL);
  assert(fifo != NULL);

  hp = malloc(sizeof(struct tclhttpd_st));
  if(hp == NULL)
    return(NULL);

  hp->script = NULL;
  hp->fifo = NULL;
  hp->child_status = -1;
  hp->child_pid = -1;

  if(set_tclhttpd_script(hp, script) != 0){
    free(hp);
    return(NULL);
  }

  if(set_tclhttpd_fifo(hp, fifo) != 0){
    free(hp->script);
    free(hp);
    return(NULL);
  }

  if(status == 0)
    status = tclhttpd_create_child(hp);

  if(status != 0){
    tclhttpd_close(hp);
    hp = NULL;
  }

  return(hp);
}

void tclhttpd_close(struct tclhttpd_st *hp){

  int fifo_fd = -1;
  char stop = 1;
  int status = 0;
  int wait_status = 0;
  int exit_status = 0;
  int signo = 0;
  int i;

  assert(hp != NULL);

  if(hp->child_pid == -1)
    goto End;

  /*
   * It is possible that this function is called before the tclhttpd child
   * has had chance to start and create the fifo. It can happen, for example,
   * if there were initialization errors in main after the tclhttpd server
   * was spawned. Therefore, if the fifo does not yet exist, we wait some time
   * retrying before giving up.
   */
  i = 1;
  while(((status = file_exists(hp->fifo)) != 0) &&
	(i <= TCLHTTPD_CLOSE_RETRY)){
    ++i;
    sleep(TCLHTTPD_CLOSE_SLEEP);
  }

  if(status == 0){
    if((fifo_fd = open(hp->fifo, O_WRONLY)) == -1)
      status = -1;
  }

  if(fifo_fd != -1){
    if(write(fifo_fd, &stop, sizeof(char)) == -1)
      status = -1;

    close(fifo_fd);
  }

  if(status != 0)
    goto End;

  i = 1;
  while((status == 0) && (i <= TCLHTTPD_CLOSE_RETRY)){
    sleep(TCLHTTPD_CLOSE_SLEEP);
    status = waitpid(hp->child_pid, &wait_status, WNOHANG);
    if(status == 0)
      (void)kill(hp->child_pid, SIGTERM);

    ++i;
  }

  if(status == 0)
    log_errx("Cannot terminate %s.", hp->script);
  else if(status > 0){
    if(WIFEXITED(wait_status))
      exit_status = WEXITSTATUS(wait_status);
    else if(WIFSIGNALED(wait_status)){
      exit_status = 1;
      signo =  WTERMSIG(wait_status);	/* signal number */
    }

    if(exit_status != 0){
      if(signo != 0){
	log_errx("%s exited abnormally with signal %d.", hp->script, signo);
      }else{
	log_errx("%s exited with error %d.", hp->script, exit_status);
      }
    }else
      log_info("Finished http server.");
  }

 End:
  
  if(hp->script != NULL)
    free(hp->script);

  if(hp->fifo != NULL)
    free(hp->fifo);

  free(hp);
}

static int set_tclhttpd_script(struct tclhttpd_st *hp, char *script){

  int status = 0;
  char *s;
  int length;

  assert((script != NULL) && (script[0] != '\0'));

  length = strlen(script);
  s = malloc(length + 1);
  if(s == NULL)
    return(-1);

  if(hp->script != NULL)
    free(hp->script);
  
  hp->script = s;
  strncpy(hp->script, script, length + 1);

  return(status);
}

static int set_tclhttpd_fifo(struct tclhttpd_st *hp, char *fifo){

  int status = 0;
  char *s;
  int length;

  assert((fifo != NULL) && (fifo[0] != '\0'));

  length = strlen(fifo);
  s = malloc(length + 1);
  if(s == NULL)
    return(-1);

  if(hp->fifo != NULL)
    free(hp->fifo);
  
  hp->fifo = s;
  strncpy(hp->fifo, fifo, length + 1);

  return(status);
}

static int tclhttpd_create_child(struct tclhttpd_st *hp){

  int status = 0;
  pid_t pid;

  pid = fork();
  if(pid == -1){
    log_err("Cannot spawn http server.");
    return(-1);
  }
  
  if(pid == 0){
    /*
     * The child spawns the http server, passing the name of the
     * fifo as argument to the tclhttpd script.
     */
    status = execl(hp->script, hp->script, hp->fifo, (char*)0);
    if(status == -1)
      log_err2("Cannot execute", hp->script);

    _exit(1);
  }

  hp->child_status = 0;
  hp->child_pid = pid;

  return(status);
}

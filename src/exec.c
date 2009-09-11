/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "err.h"
#include "util.h"
#include "file.h"
#include "globals.h"
#include "tclevalfile.h"
#include "exec.h"

/*
 * The start and stop script are executed within a tcl interpreter. That
 * is ok for those scripts, which will be evaluated once, and which may
 * need to modify the environment depending on the application.
 * The scheduler is executed repeatedly, and it should not be executed
 * in the context of a tcl interpreter with access to the complete
 * environment. Either in a safe interpreter (but that limits what it can do)
 * or via traditional forks, which is the route we follow here.
 */
static void exec_external_program(char *s);
static void wait_first_child(pid_t child_pid, char *name);

int exec_startscript(void){

  char *s = g.startscript;
  int status = 0;

  if(valid_str(s) == 0)
    return(0);
  
  if(file_exists(s) != 0){
    log_errx("Start script %s not found.", s);
    return(1);
  }

  status = tcl_eval_file(s, NULL);

  return(status);
}

int exec_stopscript(void){

  char *s = g.stopscript;
  int status = 0;

  if(valid_str(s) == 0)
    return(0);
  
  if(file_exists(s) != 0){
    log_errx("Stop script %s not found.", s);
    return(1);
  }

  status = tcl_eval_file(s, NULL);

  return(status);
}

void exec_scheduler(void){

  char *s = g.scheduler;

  if(valid_str(s) == 0){
    log_errx("No scheduler defined.");
    return;
  }

  exec_external_program(s);
}

static void exec_external_program(char *s){

  pid_t pid;
  int status;

  if(file_exists(s) != 0){
    log_errx("%s not found.", s);
    return;
  }

  pid = fork();
  if(pid == -1){
    log_err2("Error executing", s);
    return;
  }
 
  if(pid == 0){
    /*
     * First child forks again.
     */
    pid = fork();
    if(pid == -1){
      log_err2("Error executing", s);
      _exit(1);
    }else if(pid > 0){
      /*
       * Parent of second fork (first child).
       */
      _exit(0);
    }
    
    /*
     * This is now the second child.
     */
    log_verbose(1, "Executing %s", s);
    status = execl(s, s, NULL);
    if(status == -1)
      log_err2("Cannot exec", s);

    _exit(1);
  }

  /* This is the master parent. Wait for the first child. */
  wait_first_child(pid, s);  
}

static void wait_first_child(pid_t child_pid, char *name){

  int wait_status = 0;
  int exit_status = 0;
  int signo = 0;
  pid_t pid;

  pid = waitpid(child_pid, &wait_status, 0);
  if(pid <= 0)
    return;
  
  /*
   * A child was awaited.
   */
  if(WIFEXITED(wait_status))
    exit_status = WEXITSTATUS(wait_status);
  else if(WIFSIGNALED(wait_status)){
    exit_status = 1;
    signo =  WTERMSIG(wait_status);	/* signal number */
  }

  if(exit_status != 0){
    if(signo != 0){
      log_errx("%s[%d] exited abnormally with signal %d.", name, pid, signo);
    }else{
      log_errx("%s[%d] exited with error %d.", name, pid, exit_status);
    }
  }
}

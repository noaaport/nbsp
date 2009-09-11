/*
 * Copyright (c) 2005-2009 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <stdlib.h>

/*
 * Various filters need to execute periodically a scheduler script.
 * Using tcl exec & seems to give problems (in debian and centos)
 * when the scheduler executed other scripts using tclgrads (expect).
 * The motivation for this program is to be able to execute the
 * the schedulers outside of the environmemt of the filter to avoid
 * those problems (which we are not sure of the origin).
 */
static void wait_first_child(pid_t child_pid, char *name);

int main(int argc, char **argv){

  pid_t pid;
  int status;
  char *progname;

  /* Shift the argv to point to the program and arguments to be executed. */
  ++argv;
  --argc;	/* not used */
  progname = argv[0];

  pid = fork();
  if(pid == -1){
    err(1, "Error executing %s", progname);
  }
 
  if(pid == 0){
    /*
     * First child forks again.
     */
    pid = fork();
    if(pid == -1){
      warn("Error executing %s", progname);
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
    fcntl(STDIN_FILENO, F_SETFD, FD_CLOEXEC);
    fcntl(STDOUT_FILENO, F_SETFD, FD_CLOEXEC);
    fcntl(STDERR_FILENO, F_SETFD, FD_CLOEXEC);

    status = execvp(progname, argv);
    if(status == -1)
      warn("Cannot exec %s", progname);

    _exit(1);
  }

  /* This is the master parent. Wait for the first child. */
  wait_first_child(pid, progname);

  return(0);
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
    if(signo != 0)
      errx(1, "%s[%d] exited abnormally with signal %d.", name, pid, signo);
    else
      errx(1, "%s[%d] exited with error %d.", name, pid, exit_status);
  }
}

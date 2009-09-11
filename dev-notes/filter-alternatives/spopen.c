/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <paths.h>
#include "spopen.h"

struct pipe_st {
  FILE *fp[2];
  pid_t pid;
};

struct pipe_list_st {
  int n;
  int nmax;
  struct pipe_st *pipes;
};

#define INITIAL_SIZE		1
#define GROW_FACTOR		2

static struct pipe_list_st gpipelist = {0, 0, NULL};
static pthread_mutex_t gpipelist_mutex = PTHREAD_MUTEX_INITIALIZER;

static int init_pipe_list(void);
static int grow_list(void);
static int find_empty_entry(void);
static int find_entry(pid_t pid);
static int add_pipe(FILE *fp[2], pid_t pid);
static void delete_pipe(int i);
static void close_all_pipes(void);
static void lock_pipelist(void);
static void unlock_pipelist(void);
void kill_pipe_list(void);

int spopen(char *command, char *type, FILE *fp[2], pid_t *pid){

  int has_1 = 0;
  int has_2 = 0;
  int fd1[2], fd2[2]; /* Parent reads from fd2 and wriotes to fd1 */
  pid_t childpid;
  char *argv[4];
  int status = 0;

  fp[0] = NULL;
  fp[1] = NULL;

  argv[0] = "sh";
  argv[1] = "-c";
  argv[2] = command;
  argv[3] = NULL;

  if(type[0] == 'r'){
    has_2 = 1;
    if(type[1] == '+')
      has_1 = 1;
    else if(type[1] != '\0')
      status = -1;
  }else if(type[0] == 'w'){
    has_1 = 1;
    if(type[1] != '\0')
      status = -1;
  }

  if(status == -1){
    errno = EINVAL;
    return(-1);
  }

  if(has_1){
    if(pipe(fd1) < 0)
      return(-1);
  }

  if(has_2){
    if(pipe(fd2) < 0){
      if(has_1){
	close(fd1[0]);
	close(fd1[1]);
      }  
      return(-1);
    }
  }

  lock_pipelist();

  if(gpipelist.n == gpipelist.nmax)
    status = grow_list();

  if(status == 0){
    if((childpid = vfork()) < 0)
      status = -1;
  }

  if(status != 0){
    if(has_1){
      close(fd1[0]);
      close(fd1[1]);
    }

    if(has_2){
      close(fd2[0]);
      close(fd2[1]);
    }  
 
    unlock_pipelist();  
    return(-1);
  }

  if(childpid == 0){ 
    /*
     * Child reads from fd1 and writes to fd2
     */
    if(has_1){
      close(fd1[1]);
      if(fd1[0] != STDIN_FILENO){
	dup2(fd1[0], STDIN_FILENO);
	close(fd1[0]);
      }
    }

    if(has_2){
      close(fd2[0]);
      if(fd2[1] != STDOUT_FILENO){
	dup2(fd2[1], STDOUT_FILENO);
	close(fd2[1]);
      }
    }

    /*
     * All pipes that are still opened from previous calls must be closed
     * in the child (APUE, p. 437).
     */
    close_all_pipes();

    execv(_PATH_BSHELL, argv);
    /*
     * In case of an error, exit but don't call atexit() functions, etc
     */
    _exit(127);			
  }

  /*
   * Parent reads from fd2 and writes to fd1
   */
  *pid = childpid;
  if(has_1){
    close(fd1[0]);
    fp[1] = fdopen(fd1[1], "w");
  }

  if(has_2){
    close(fd2[1]);
    fp[0] = fdopen(fd2[0], "r");
  }

  add_pipe(fp, childpid);

  unlock_pipelist();

  return(status);
}

int spclose(FILE *fp[2], pid_t pid){

  int status = 0;
  int wstatus;
  int i;

  lock_pipelist();
  i = find_entry(pid);
  if(i == -1){
    errno = EBADF;
    unlock_pipelist();
    return(-1);
  }

  delete_pipe(i);
  unlock_pipelist();

  if(fp[0] != NULL){
    if(fclose(fp[0]) == EOF)
      status = -1;
  }

  if(fp[1] != NULL){
    if(fclose(fp[1]) == EOF)
      status = -1;
  }

  /*
   * What happens if the filter by some reason, including an error,
   * does not quit even after we close the pipe? If we simply call
   * wait() here, we can stay here forever. One option is to
   * to send a kill(pid, SIGKILL), perhaps after waiting a few seconds,
   * or call wait with the WNOHANG option. We will so as in 
   * the standard popen().
   */

  /*
   * kill(pid, SIGKILL);
   */

  while(waitpid(pid, &wstatus, 0) < 0){
    if(errno != EINTR)
      return(-1);
  }

  return(status);
}


/*
 * These are the functions for managing the list.
 */
static int init_pipe_list(void){

  int i;

  gpipelist.pipes = malloc(INITIAL_SIZE * sizeof(struct pipe_st));

  if(gpipelist.pipes == NULL)
    return(-1);
  else
    gpipelist.nmax = INITIAL_SIZE;

  for(i = 0; i <= gpipelist.nmax - 1; ++i){
    gpipelist.pipes[i].fp[0] = NULL;
    gpipelist.pipes[i].fp[1] = NULL;
    gpipelist.pipes[i].pid = -1;
  }

  return(0);
}

void kill_pipe_list(void){

  int i;

  if(gpipelist.pipes == NULL)
    return;

  for(i = 0; i <= gpipelist.nmax - 1; ++i)
    delete_pipe(i);

  free(gpipelist.pipes);

  gpipelist.n = 0;
  gpipelist.nmax = 0;
  gpipelist.pipes = NULL;
}

static int grow_list(void){
  /*
   * Returns 0 if there are no errors or -1 otherwise.
   */
  struct pipe_st *p;
  int old_nmax;
  int newsize;
  int i;

  if(gpipelist.nmax == 0)
    return(init_pipe_list());

  old_nmax = gpipelist.nmax;
  newsize = (gpipelist.nmax * GROW_FACTOR)*sizeof(struct pipe_st);
  p = realloc(gpipelist.pipes, newsize);
  if(p == NULL)
    return(-1);

  gpipelist.nmax *= GROW_FACTOR;
  gpipelist.pipes = p;

  for(i = old_nmax; i <= gpipelist.nmax - 1; ++i){
    gpipelist.pipes[i].fp[0] = NULL;
    gpipelist.pipes[i].fp[1] = NULL;
    gpipelist.pipes[i].pid = -1;
  }

  return(0);
}  

static int add_pipe(FILE *fp[2], pid_t pid){

  int status = 0;
  int i;

  i = find_empty_entry();
  assert(i != -1);

  gpipelist.pipes[i].fp[0] = fp[0];
  gpipelist.pipes[i].fp[1] = fp[1];
  gpipelist.pipes[i].pid = pid;
  ++gpipelist.n;

  return(status);
}

static int find_empty_entry(void){
  /*
   * This function finds the first empty slot.
   */
  int i;

  for(i = 0; i <= gpipelist.nmax - 1; ++i){
    if(gpipelist.pipes[i].pid == -1)
      return(i);
  }

  return(-1);
}

static int find_entry(pid_t pid){
  /*
   * This function checks to see if the pid is already in the list.
   */
  int i;

  for(i = 0; i <= gpipelist.nmax - 1; ++i){
    if(gpipelist.pipes[i].pid == pid)
       return(i);
  }

  return(-1);
}

static void delete_pipe(int i){

  gpipelist.pipes[i].fp[0] = NULL;
  gpipelist.pipes[i].fp[1] = NULL;
  gpipelist.pipes[i].pid = -1;
  --gpipelist.n;
}

static void close_all_pipes(void){
  /*
   * This function is meant to be called by the dpopen() function
   * to close in the child all the pipes that had been opened
   * in previous calls.
   */
  int i;

  for(i = 0; i <= gpipelist.nmax - 1; ++i){
    if(gpipelist.pipes[i].pid != -1){
      if(gpipelist.pipes[i].fp[0] != NULL)
	close(fileno(gpipelist.pipes[i].fp[0]));

      if(gpipelist.pipes[i].fp[1] != NULL)
	close(fileno(gpipelist.pipes[i].fp[1]));
    }
  }
}

static void lock_pipelist(void){

  pthread_mutex_lock(&gpipelist_mutex);
}

static void unlock_pipelist(void){

  pthread_mutex_unlock(&gpipelist_mutex);
}

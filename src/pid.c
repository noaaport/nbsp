/*
 * Copyright (c) 2004-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "file.h"
#include "pid.h"

static pid_t read_pid(char *pidfile);

int create_pidfile(char *name){
  /* 
   * Creates the pid file. 
   * Returns:
   * 0 ==> no error
   * 1 ==> pid file exists (background process running ?)
   * 2 ==> could not write to the file.
   */
  int fd = -1;
  pid_t pid;
  intmax_t pidmax;
  FILE *f = NULL;
  mode_t mode = S_IRUSR + S_IWUSR + S_IRGRP + S_IROTH;
  int status = 0;

  fd = open(name, O_RDWR | O_CREAT | O_EXCL,mode);
  if(fd == -1){
    return(1);
  }else{
    close(fd);
  }

  f = fopen(name, "w");
  status = (f == NULL);

  if(status == 0){
    pid = getpid();
    pidmax = pid;
    if(fprintf(f, "%" PRIdMAX "\n", pidmax) < 0){
      status = 2;
      unlink(name);
    }
    fclose(f);
  }

  return(status);
}

int remove_pidfile(char *name){
  /* 
   * Returns:
   * 0 ==> removed pid file, or it is not our pid file
   * 1 ==> no pid file
   * 2 ==> could not remove it
   */
  int status = 0;
  pid_t pid;

  pid = read_pid(name);
  if(pid == 0){
    /* 
     * no pid file ==> we could not create it at initialization
     */
    status = 1;
  }else if(pid == -1){
    status = 2;
  }else if(pid == getpid()){
    /* 
     * it is ours 
     */
   if(unlink(name) != 0)
     status = 2;
  }

  return(status);
}

static pid_t read_pid(char *pidfile){
  /* 
   * This function checks to see if the background process is
   * running and tries to read the pid stored in the pid file.
   * If the file pid file does not exist,
   * we assume that the daemon is not running and return(0). If the
   * the file exists but could not be read, return(-1). Otherwise it returns
   * the pid of the daemon.
   */

  FILE *fp = NULL;
  pid_t pid = -1;
  intmax_t pidmax;
  int status = 0;
  
  status = file_exists(pidfile);
  if(status == 1){
    /* 
     * the file does not exist.
     */
    return(0);
  }else if(status == -1){
    /* 
     * some error from stat
     */
    return(-1);
  }

  status = ((fp = fopen(pidfile, "r")) == NULL);
  if(status == 0){
      status = (fscanf(fp, "%" PRIdMAX, &pidmax) == EOF);
      fclose(fp);
  }

  if(status == 0)
    pid = (pid_t)pidmax;

  return(pid);
}


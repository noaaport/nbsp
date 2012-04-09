/*
 * Copyright (c) 2005-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "readn.h"
#include "pfilter.h"

static struct pfilter_st *pfilter_open_script(char *script, char *fifo);

struct pfilter_st *pfilter_open(char *script){

  return(pfilter_open_script(script, NULL));
}

struct pfilter_st *pfilter_open_wr(char *script, char *fifo,
				   unsigned int secs){
  int status = 0;
  struct pfilter_st *filterp;
  int fd;
 
  filterp = pfilter_open_script(script, fifo);
  if(filterp == NULL)
    return(NULL);

  (void)unlink(fifo);
  status = mkfifo(fifo, 0600);
  if(status == -1){
    pfilter_close(filterp);
    return(NULL);
  }

  fd = open(fifo, O_RDWR | O_NONBLOCK);
  if(fd == -1){
    (void)unlink(fifo);
    pfilter_close(filterp);
    return(NULL);
  }

  filterp->fifofd = fd;
  filterp->fifofpath = fifo;
  filterp->secs = secs;

  return(filterp);
}

void pfilter_close(struct pfilter_st *filterp){

  assert(filterp != NULL);

  if(filterp->fp != NULL)
    (void)pclose(filterp->fp);

  if(filterp->fifofd != -1)
    (void)close(filterp->fifofd);
		
  if(filterp->fifofpath != NULL)
    (void)unlink(filterp->fifofpath);

  free(filterp);
}

int pfilter_read(struct pfilter_st *filterp,
		 void *buffer, size_t size){
  /*
   * Reading is done from the fifo.
   */
  ssize_t n = 0;

  n = readn_fifo(filterp->fifofd, buffer, size, filterp->secs);
  if(n == -1)
    return(-1);
  else if((size_t)n < size)
    return(1);

  return(0);
}

/*
 * local functions
 */
static struct pfilter_st *pfilter_open_script(char *script, char *fifo){

  struct pfilter_st *filterp;
  char *cmd;
  int cmdlen;
  int n;

  assert((script != NULL) && (script[0] != '\0'));

  cmdlen = strlen(script);
  if(fifo != NULL){
    cmdlen += strlen(fifo) + 1;  /* include the space */
  }
  cmd = malloc(cmdlen + 1);	/* include '\0' */
  if(cmd == NULL)
    return(NULL);

  if(fifo == NULL)
    n = snprintf(cmd, cmdlen + 1, "%s", script);
  else 
    n = snprintf(cmd, cmdlen + 1, "%s %s", script, fifo);

  assert(cmdlen == n);

 filterp = malloc(sizeof(struct pfilter_st));
  if(filterp == NULL)
    return(NULL);

  filterp->fifofd = -1;
  filterp->fifofpath = NULL;
  filterp->secs = 0;
  
  filterp->fp = popen(cmd, "w");
  if(filterp->fp == NULL){
    free(filterp);
    filterp = NULL;
  }

  free(cmd);

  return(filterp);
}

int pfilter_flush(struct pfilter_st *filterp){

  int status;

  status = fflush(filterp->fp);

  return(status);
}

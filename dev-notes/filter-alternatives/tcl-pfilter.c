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
static int tcl_writen_chars(Tcl_Channel channel, void *buffer, size_t n);

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

  if(filterp->printf_buffer != NULL)
    free(filterp->printf_buffer);

  if(filterp->channel != NULL)
    Tcl_Close(filterp->interp, filterp->channel);

  if(filterp->interp != NULL)
    Tcl_DeleteInterp(filterp->interp);

  if(filterp->fifofd != -1)
    (void)close(filterp->fifofd);
		
  if(filterp->fifofpath != NULL)
    (void)unlink(filterp->fifofpath);

  free(filterp);
}

int pfilter_write(struct pfilter_st *filterp, void *buffer, size_t size){

  int n;

  n = tcl_writen_chars(filterp->channel, buffer, size);

  return(n);
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

int pfilter_vprintf(struct pfilter_st *filterp, char *format, ...){
  /*
   * Returns -1, or the number of characters written.
   */
  int n;
  char *buffer;
  int size;
  va_list ap;

  buffer = filterp->printf_buffer;
  size = filterp->printf_buffer_size;

  /*
   * The returned value in n does _not_ include the traling \0 that will
   * will be added by the function.
   */
  va_start(ap, format);
  n = vsnprintf(buffer, (size_t)size, format, ap);
  va_end(ap);

  if(n >= size){
    while(n >= size){
      size *= PRINTF_BUFFER_SIZE_GROW_FACTOR;
    }

    buffer = malloc(size);
    if(buffer == NULL)
      return(-1);

    free(filterp->printf_buffer);

    filterp->printf_buffer = buffer;
    filterp->printf_buffer_size = size;

    va_start(ap, format);
    n = vsnprintf(buffer, size, format, ap);
    va_end(ap);
  }

  n = pfilter_write(filterp, buffer, (size_t)n);

  return(n);
}

int pfilter_flush(struct pfilter_st *filterp){

  if(Tcl_Flush(filterp->channel) != TCL_OK)
    return(-1);
  
  return(0);
}

/*
 * local functions
 */
static struct pfilter_st *pfilter_open_script(char *script, char *fifo){

  struct pfilter_st *filterp;
  Tcl_Interp *interp;
  Tcl_Channel channel;
  char *buffer;
  int argc;
  const char *argv[3];
  int flags = PFILTER_FLAGS;
  int status = 0;

  argc = 1;
  argv[1] = NULL;
  argv[2] = NULL;

  assert((script != NULL) && (script[0] != '\0'));
  argv[0] = script;

  if(fifo != NULL){
    assert(fifo[0] != '\0');
    argv[1] = fifo;
    ++argc;
  }

 filterp = malloc(sizeof(struct pfilter_st));
  if(filterp == NULL)
    return(NULL);

  filterp->interp = NULL;
  filterp->channel = NULL;
  filterp->printf_buffer = NULL;
  filterp->printf_buffer_size = 0;
  filterp->fifofd = -1;
  filterp->fifofpath = NULL;
  filterp->secs = 0;

  interp = Tcl_CreateInterp();
  if(interp == NULL){
    free(filterp);
    return(NULL);
  }

  channel = Tcl_OpenCommandChannel(interp, argc, argv, flags);  
  if(channel == NULL){
    Tcl_DeleteInterp(interp);
    free(filterp);
    return(NULL);
  }

  /*
   * Sun Mar 15 14:10:08 AST 2009
   *
   * In FreeBSD 7.1 there were consistent core dumps like this
   * #0  0x0000000800de9959 in free () from /lib/libc.so.7
   * #1  0x00000008008e5748 in TclFreeObj () from /usr/local/lib/libtcl84.so.1
   * #2  0x00000008008d3c08 in DoWriteChars ()from /usr/local/lib/libtcl84.so.1
   * #3  0x0000000000414f1a in pfilter_write ()
   * #4  0x000000000041500c in pfilter_vprintf ()
   * #5  0x0000000000411e34 in server_main ()
   * #6  0x0000000800c6ba27 in pthread_getprio () from /lib/libthr.so.3
   * #7  0x0000000000000000 in ?? ()
   *
   * always comming when writing to the ewinfilter. I don't understand why.
   * It seems to have to do with the encoding/decoding stuff in
   * DoWriteChars in tclIO.c. Dor this reason I have decided to
   * comment out the setting below. This seems to solve the issue.
   * Anyway what the server sends to the filters is ascii, and in addition
   * it avoid a lot of overhead of the tcl library functions.
   *
   * status = Tcl_SetChannelOption(interp, channel, "-encoding", "binary");
   * if(status == TCL_OK)
   * status = Tcl_SetChannelOption(interp, channel, "-translation", "binary");
   */

  if(status != TCL_OK){
    Tcl_Close(interp, channel);
    Tcl_DeleteInterp(interp);
    free(filterp);
    return(NULL);
  }

  buffer = malloc(PRINTF_BUFFER_SIZE_INIT);
  if(buffer == NULL){
    Tcl_Close(interp, channel);
    Tcl_DeleteInterp(interp);
    free(filterp);
    return(NULL);
  }

  filterp->interp = interp;
  filterp->channel = channel;
  filterp->printf_buffer = buffer;
  filterp->printf_buffer_size = PRINTF_BUFFER_SIZE_INIT;
  
  return(filterp);
}

static int tcl_writen_chars(Tcl_Channel channel, void *buffer, size_t n) {

  size_t nleft;
  ssize_t nwritten;
  const char *p;

  p = (char*)buffer;
  nleft = n;
  while(nleft > 0) {
    nwritten = Tcl_WriteChars(channel, p, nleft);
    if(nwritten < 0)
      return(-1);

    nleft -= nwritten;
    p += nwritten;
  }

  return(n);
}

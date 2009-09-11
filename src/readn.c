/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include "readn.h"

ssize_t read1(int fd, void *buf, size_t size, unsigned int msecs, int retry){
  /*
   * Returns:
   *   the number of characters that were read (0 ==> eof).
   *   -1 if there was a system error (from poll or read), 
   *   -2 if (poll) timedout. 
   *
   * In FreeBSD (at least up to 7.2), poll does not consider a FIFO
   * ready on eof. So, for a FIFO, the read() will never return
   * 0 here (either -1 or n > 0). But for network sockets, read() can return 0
   * because poll considers it ready if it has closed. Since we want to
   * know when the read() returns zero and when poll timed out,
   * we use the -2 for the latter case.
   * In Linux (and if FreeBSD removes the bug), FIFO's behave similarly.
   * Since 0 will never be returned for FIFOs in FreeBSD, there
   * the eof condition for FIFOs must be detected by calling read()
   * after this read1() returns -2; that read() should then return 0.
   */
  int status = 0;
  ssize_t n = 0;			/* number of bytes read */
  struct pollfd pfd;
  int i;

  pfd.fd = fd;
  pfd.events = POLLIN;

  if(retry < 0)
    retry = 0;

  for(i = 0; (i <= retry) && (status == 0); ++i){
    status = poll(&pfd, 1, msecs);
  }

  if(status == 0)
    n = -2;
  else if(status == -1)
    n = -1;
  else if((pfd.revents & POLLIN) != 0){
    /*
     * This read() returns either -1 or a positive number for fifos
     * but it can be zero for network sockets.
     */
    n = read(fd, buf, size);
  }

  return(n);
}

ssize_t write1(int fd, void *buf, size_t size, unsigned int msecs, int retry){
  /*
   * Returns:
   * -1 for error
   *  0 for timed out
   *  number of characters written
   */
  int status = 0;
  ssize_t n = 0;			/* number of bytes written */
  struct pollfd pfd;
  int i;

  pfd.fd = fd;
  pfd.events = POLLOUT;

  if(retry < 0)
    retry = 0;

  for(i = 0; (i <= retry) && (status == 0); ++i){
    status = poll(&pfd, 1, msecs);
  }

  if(status == -1)
    n = -1;
  else if((status > 0) && ((pfd.revents & POLLOUT) != 0))
    n = write(fd, buf, size);

  return(n);
}

ssize_t sreadm(int fd, void *buff, size_t size,
	       unsigned int msecs, int retry, int *eof){
  /*
   * Returns:
   * 
   * -1 if a real error
   * -2 if timed out before anything could be read (poll timed out)
   * n >= 0 number of characters read (includes partial read if poll timed out
   *  or disconnection ocurred while reading, and 0 if disconnection
   *  was detected before reading anything). In any of these cases (n >= 0)
   *  when eof is detected the *eof argument is set to 1 or 0 otherwise
   *  (if it is not NULL).
   */
  size_t i = 0;
  ssize_t n;
  char *p = (char*)buff;

  while(i < size){    
    if((n = read1(fd, &p[i], size - i, msecs, retry)) < 0){
      if((n == -2) && (i > 0)){
	/*
	 * The break makes the function return a partial read.
	 * Indicate that it is due to poll timeout and not eof.
	 */
	if(eof != NULL){
	  *eof = 0;
	}
	break;
      }else
	return(n);
    }else if(n == 0){
      /*
       * eof (can happen only for sockets).
       */
      if(eof != NULL){
	*eof = 1;
      }
      break;
    }else{
      i += n;
    }
  }

  return(i);
}

ssize_t readm(int fd, void *buff, size_t size, unsigned int msecs, int retry){

  return(sreadm(fd, buff, size, msecs, retry, NULL));
}

ssize_t writem(int fd, void *buf, size_t size, unsigned int msecs, int retry){

  ssize_t n;			/* number of bytes in one write */
  size_t i = 0;			/* accumulated number of bytes writen */
  char *p = (char*)buf;

  while(i < size){
    if((n = write1(fd, &p[i], size - i, msecs, retry)) == -1)
      return(-1);
    else if(n == 0)
      break;
    else
      i += n;
  }

  return(i);
}

ssize_t sreadn(int fd, void *buf, size_t size,
	       unsigned int secs, int retry, int *eof){

  return(sreadm(fd, buf, size, 1000*secs, retry, eof));
}

ssize_t readn(int fd, void *buff, size_t size, unsigned int secs, int retry){

  return(sreadm(fd, buff, size, 1000*secs, retry, NULL));
}

ssize_t writen(int fd, void *buf,size_t size, unsigned int secs, int retry){

  return(writem(fd, buf, size, 1000*secs, retry));
}

ssize_t readn_fifo(int fd, void *buf, size_t size, unsigned int secs){
  /*
   * This function is meant to be used for fifos that are open
   * with the O_NONBLOCK flag. Since the read() and readm() functions
   * will not return 0 on eof for fifos, but -2 because poll will timeout,
   * here we make another read() without calling poll.
   */ 
  ssize_t n;

  n = readn(fd, buf, size, secs, 0);
  if(n == -2){
    if(read(fd, buf, size) == 0)
      n = 0;
  }

  return(n);
}

ssize_t readm_fifo(int fd, void *buf, size_t size, unsigned int msecs){
  /*
   * Similar as above.
   */ 
  ssize_t n;

  n = readm(fd, buf, size, msecs, 0);
  if(n == -2){
    if(read(fd, buf, size) == 0)
      n = 0;
  }

  return(n);
}

ssize_t dpgets(int fd, char *buf, size_t size){
  /*
   * Behaves similar to fgets(), and returns the same values as read().
   */
  size_t b = 0;
  char c;
  ssize_t n;

  while(b + 1 < size){
    n = read(fd, &c, 1);
    if(n < 0)
      return(-1);
    else if(n == 0)
      break;
    else{
      buf[b++] = c;
      if(c == '\n')
	break;
    }
  }

  buf[b] = '\0';

  return(b);
}

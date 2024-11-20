/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include "io.h"

ssize_t writef(int fd, void *buf, size_t size) {
  /*
   * To be used when output is a (the npemwin) fifo
   */
  ssize_t n;		/* number of bytes in one write */
  size_t i = 0;		/* accumulated number of bytes writen */
  char *p = (char*)buf;

  while(i < size){
    if((n = write(fd, &p[i], size - i)) == -1)
      return(-1);
    else if(n == 0)
      break;
    else
      i += n;
  }

  return(i);
}

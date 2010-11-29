/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <unistd.h>
#include "misc.h"

int read_skip_count(int fd, size_t count, char *buffer, size_t buffer_size){

  size_t nleft;
  size_t ntry;
  int nread;

  nleft = count;
  while(nleft > 0){
    if(nleft > buffer_size)
      ntry = buffer_size;
    else
      ntry = nleft;

    nread = read(fd, buffer, ntry);
    if(nread == -1)
      return(-1);
    else if((size_t)nread != ntry)
      return(1);

    nleft -= nread;
  }

  return(0);
}

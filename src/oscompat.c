/*
 * Copyright (c) 2005-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <time.h>
#include <sys/time.h>
#include "oscompat.h"

#ifdef HAS_CLOCK_GETTIME
int oscompat_clock_gettime(struct timespec *tp){

  int status;

  status = clock_gettime(CLOCK_REALTIME, tp);

  return(status);
}
#else
int oscompat_clock_gettime(struct timespec *tp){

  int status;
  struct timeval tv;

  status = gettimeofday(&tv, NULL);
  if(status != 0)
    return(status);

  tp->tv_sec = tv.tv_sec;
  tp->tv_nsec = (long)tv.tv_usec * 1000;

  return(status);
}
#endif

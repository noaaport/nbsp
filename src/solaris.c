/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

/*
 * This file contains library functions that I use in freebsd but are not
 * in solaris or linux. 
 */
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "solaris.h"

#ifndef HAVE_DAEMON
int daemon(int nochdir, int noclose){

  int status = 0;
  int i;

  status = fork();
  if(status == -1)
    return(status);
  else if(status > 0)
    exit(0);

  /*
   * Only the child continues
   */

  if(setsid() == -1)
    return(-1);

  /* 
   * umask(0);
   *
   * we leave the umask as inherited from the user that starts the program
   */

  if(nochdir == 0)
    chdir("/");

  if(noclose == 0){
    for(i = 0; i <= 2; ++i){
      if(status == 0)
	status = close(i);
      else
	break;
    }
  }

  return(status);
}
#endif

#ifndef HAVE_ERR
void err(int eval, const char *fmt, ...){

  va_list ap;

  va_start(ap,fmt);
  vfprintf(stderr,fmt,ap);
  va_end(ap);

  fprintf(stderr," %s\n",strerror(errno));

  fflush(stderr);

  exit(eval);
}

void errx(int eval, const char *fmt, ...){

  va_list ap;

  va_start(ap,fmt);
  vfprintf(stderr,fmt,ap);
  va_end(ap);

  fflush(stderr);

  exit(eval);
}

void warn(const char *fmt, ...){

  va_list ap;

  va_start(ap,fmt);
  vfprintf(stderr,fmt,ap);
  va_end(ap);

  fprintf(stderr," %s\n",strerror(errno));

  fflush(stderr);
}

void warnx(const char *fmt, ...){

  va_list ap;

  va_start(ap,fmt);
  vfprintf(stderr,fmt,ap);
  va_end(ap);

  fflush(stderr);
}

#endif




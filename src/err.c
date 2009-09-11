/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <unistd.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>	/* exit() for log_assert() */
#include <db.h>
#include "err.h"

static int f_daemon = 0;
static int f_verbose = 0;
static int f_debug = 0;

void set_log_daemon(void){

  f_daemon = 1;
}

void set_log_debug(int flag){

  f_debug = flag;
}

void set_log_verbose(int flag){

  f_verbose = flag;
}

void log_msg(int priority, char *fmt, ...){

  va_list ap;

  va_start(ap, fmt);

  if(f_daemon == 1)
    vsyslog(priority, fmt, ap);
  else{
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
  }

  va_end(ap);
}

void log_info(char *fmt, ...){

  va_list ap;

  va_start(ap, fmt);

  if(f_daemon == 1)
    vsyslog(LOG_INFO, fmt, ap);
  else{
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
  }

  va_end(ap);
}

void log_warnx(char *fmt, ...){

  va_list ap;

  va_start(ap, fmt);

  if(f_daemon == 1)
    vsyslog(LOG_WARNING, fmt, ap);
  else{
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
  }

  va_end(ap);
}

void log_errx(char *fmt, ...){

  va_list ap;

  va_start(ap, fmt);

  if(f_daemon == 1)
    vsyslog(LOG_ERR, fmt, ap);
  else{    
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
  }

  va_end(ap);
}

void log_err(char *s){

  if(f_daemon == 1)
    syslog(LOG_ERR, "%s %m", s);
  else
    warn(s);
}

void log_err2(char *s1, char *s2){

  if(f_daemon == 1)
    syslog(LOG_ERR, "%s %s. %m", s1, s2);
  else
    warn("%s %s.", s1, s2);
}

void log_err2u(char *s1, unsigned int u){

  if(f_daemon == 1)
    syslog(LOG_ERR, "%s %u. %m", s1, u);
  else
    warn("%s %u.", s1, u);
}

void log_verbose(int level, char *fmt, ...){

  va_list ap;

  if(level > f_verbose)
    return;

  va_start(ap, fmt);

  if(f_daemon == 1)
    vsyslog(LOG_INFO, fmt, ap);
  else{
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
  }

  va_end(ap);
}

void log_debug(char *fmt, ...){

  va_list ap;

  if(f_debug == 0)
    return;

  va_start(ap, fmt);

  if(f_daemon == 1)
    vsyslog(LOG_DEBUG, fmt, ap);
  else{
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
  }

  va_end(ap);
}

void log_err_open(char *fname){

  log_err2("Could not open", fname);
}

void log_err_write(char *fname){

  log_err2("Could not write to", fname);
}

void log_err_read(char *fname){

  log_err2("Could not read from", fname);
}

void log_err_db(char *s, int dberror){
  /*
   * The libqdb functions that return dberror in the argument, return
   * -1 when there is a system error outside the db library or from a
   * db library function. In those cases dberror contains the value of errno
   * or the return code from the db library function.
   */
  if(dberror == 0)
    log_err(s);
  else
    log_errx("%s %s", s, db_strerror(dberror));
}

void log_err2_db(char *s1, char *s2, int dberror){

  if(dberror == 0)
    log_err2(s1, s2);
  else
    log_errx("%s %s. %s", s1, s2, db_strerror(dberror));
}

void log_assert(const char *func, const char *file, int line,
	    const char *failedexpr){
  if(func == NULL)
    log_info("Assertion failed: (%s), file %s, line %d.\n", failedexpr,
	     file, line);
  else
    log_info("Assertion failed: (%s), function %s, file %s, line %d.\n",
	     failedexpr, func, file, line);

  /* abort() */
  exit(1);
}

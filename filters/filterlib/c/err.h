/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef ERR_H
#define ERR_H

#include "config.h"

#ifdef HAVE_ERR
#include <err.h>
#else
#include "solaris.h"
#endif

void set_progname(char *ident);
void set_usesyslog(void);
void log_msg(int priority, char *fmt, ...);
void log_info(char *fmt, ...);
void log_warnx(char *fmt, ...);
void log_errx(int e, char *fmt, ...);
void log_err(int e, char *fmt, ...);
void log_err_open(char *fname);
void log_err_write(char *fname);
void log_err_read(char *fname);
void log_errn_open(char *fname);
void log_errn_write(char *fname);
void log_errn_read(char *fname);

#endif

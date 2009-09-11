/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SOLARIS_H
#define SOLARIS_H

#include "config.h"

#ifndef HAVE_DAEMON
int daemon(int nochdir, int noclose);
#endif

#ifndef HAVE_ERR
void err(int eval, const char *fmt, ...);
void errx(int eval, const char *fmt, ...);
void warn(const char *fmt, ...);
void warnx(const char *fmt, ...);
#endif

#endif


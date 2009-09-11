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

void set_log_daemon(void);
void set_log_debug(int flag);
void set_log_verbose(int flag);
void log_msg(int priority, char *fmt, ...);
void log_info(char *fmt, ...);
void log_warnx(char *fmt, ...);
void log_errx(char *fmt, ...);
void log_err(char *s);
void log_err2(char *s1, char *s2);
void log_err2u(char *s1, unsigned int u);
void log_verbose(int level, char *fmt, ...);
void log_debug(char *fmt, ...);
void log_err_open(char *fname);
void log_err_write(char *fname);
void log_err_read(char *fname);
void log_err_db(char *s, int dberror);
void log_err2_db(char *s1, char *s2, int dberror);
void log_assert(const char *func, const char *file, int line,
		const char *failedexpr);

#endif

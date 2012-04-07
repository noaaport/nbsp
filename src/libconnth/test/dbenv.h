/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef NBSP_DBENV_H
#define NBSP_DBENV_H

#include <db.h>

DB_ENV *dbenv_open(void);
void dbenv_close(DB_ENV *dbenv);

#endif

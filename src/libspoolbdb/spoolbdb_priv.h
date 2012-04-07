/*
 * Copyright (c) 2006-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SPOOLBDB_PRIV_H
#define SPOOLBDB_PRIV_H

#include <db.h>

int spoolbdb_open(DB **db, DB_ENV *dbenv,
		  char *dbfname, int dbmode,
		  int f_mpool_nofile, int f_rdonly);
int spoolbdb_close(DB *dbp);
int spoolbdb_read(DB *dbp, char *fkey, struct spoolbuf_st *spoolb);
int spoolbdb_write(DB *dbp, char *fkey, struct spoolbuf_st *spoolb);
int spoolbdb_write2(DB *dbp, char *fkey, void *data, uint32_t datasize);

#endif

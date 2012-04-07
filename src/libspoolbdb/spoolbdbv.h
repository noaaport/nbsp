/*
 * Copyright (c) 2006-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SPOOLBDBV_H
#define SPOOLBDBV_H

#include <stddef.h>
#include <pthread.h>
#include <db.h>
#include "spoolbdb.h"

/*
 * *dbv[] is a circular array of db pointers, from dbv[0] ... dbv[Ndb + 1].
 * The first and the last element stay NULL to mark the start and end of
 * the array. The wrdb is a pointer to the current element (wrdb = &dbv[i]).
 * When the current element has consumed its memory budget, wrdb is increased
 * to use the next element. The new db is truncated, n is reset to 0,
 * and then it is used until its memory saturates and the process repeats.
 * When wrdb hits the last element dbv[Ndb + 1] (which is NULL), then
 * it is reset to dvb[1]. Each db is not allowed to grow larger than 75%
 * of the memory (dbcache_mb) that the applications requests for each db.
 */

struct spoolbdbv_st {
  DB **dbv;		/* array of db handles */
  DB **wrdb;		/* pointer to current handle */
  DB_ENV *dbenv;
  uint32_t Ndb;		/* number of usable elements; dbv[1] ... dbv[Ndb] */
  uint32_t n;		/* current number of elements in current db */
  uint32_t maxsize;     /* maximum total size (bytes) of each db */
  uint32_t size;	/* current total size of each db */
  pthread_mutex_t mutex;
};

int spoolbdbv_open(struct spoolbdbv_st **spoolbdbv,
		   char *dbhome, int dbenv_mode,
		   uint32_t dbcache_mb, uint32_t maxsize_per128,
		   uint32_t Ndb, char *dbfname, int dbfile_mode,
		   int f_mpool_nofile, int f_rdonly);
int spoolbdbv_close(struct spoolbdbv_st *spoolbdbv);

int spoolbdbv_truncate(struct spoolbdbv_st *spoolbdbv);
int spoolbdbv_truncate_oldest(struct spoolbdbv_st *spoolbdbv);

int spoolbdbv_read(struct spoolbdbv_st *spoolbdbv,
		  char *fkey, struct spoolbuf_st *spoolb);
int spoolbdbv_write(struct spoolbdbv_st *spoolbdbv,
		   char *fkey, struct spoolbuf_st *spoolb);
int spoolbdbv_write2(struct spoolbdbv_st *spoolbdbv,
		    char *fkey, void *data, uint32_t datasize);

int spoolbdbv_read_unlocked(struct spoolbdbv_st *spoolbdbv,
			   char *fkey, struct spoolbuf_st *spoolb);
int spoolbdbv_write_unlocked(struct spoolbdbv_st *spoolbdbv,
			    char *fkey, struct spoolbuf_st *spoolb);
int spoolbdbv_write2_unlocked(struct spoolbdbv_st *spoolbdbv,
			     char *fkey, void *data, uint32_t datasize);

int spoolbdbv_set_event_notify(struct spoolbdbv_st *spoolbdbv,
    void (*db_event_fcn)(DB_ENV *dbenv, u_int32_t event,  void *event_info)); 

#endif

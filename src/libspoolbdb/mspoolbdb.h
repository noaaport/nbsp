/*
 * Copyright (c) 2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdint.h>
#include "spoolbdbv.h"
#include "spoolslots.h"

#ifndef MSPOOLBDB_H
#define MSPOOLBDB_H

struct mspoolbdb_st {
  struct spoolbdbv_st *spoolbdbv;
  struct spoolbuf_slots_st *spoolslots;
};

/*
 * Private memory db. If dbhome == NULL then it is a pure in-memory db,
 * otherwise the db is in memory but the environment is in the fs.
 */
int mspoolbdb_create(struct mspoolbdb_st **mspool,
		     char *dbhome, int dbenv_mode,
		     uint32_t dbcache_mb, uint32_t maxsize_per128,
		     uint32_t Ndb, int Nslots);
int mspoolbdb_destroy(struct mspoolbdb_st *mspool);
int mspoolbdb_insert(struct mspoolbdb_st *mspool,
		     char *fkey, void *fdata, uint32_t fdata_size);

/* read (slot) functions */
int mspoolbdb_slots_open(struct mspoolbdb_st *mspool, char *fkey, int *sd);
int mspoolbdb_slots_close(struct mspoolbdb_st *mspool, int sd);
size_t mspoolbdb_slots_read(struct mspoolbdb_st *mspool, int sd,
			    void *data, size_t data_size);
size_t mspoolbdb_slots_datasize(struct mspoolbdb_st *mspool, int sd);

/* truncate */
int mspoolbdb_truncate(struct mspoolbdb_st *mspool);
int mspoolbdb_truncate_oldest(struct mspoolbdb_st *mspool);

/* db_recovery notification callback */
int mspoolbdb_set_event_notify(struct mspoolbdb_st *mspool,
    void (*db_event_fcn)(DB_ENV *dbenv, u_int32_t event, void *event_info));

/*
 * shared memory
 */
int cspoolbdb_create(struct mspoolbdb_st **mspool,
		     char *dbhome, int dbenv_mode,
		     uint32_t dbcache_mb, uint32_t maxsize_per128,
		     char *dbfname, int dbfile_mode, uint32_t Ndb,
		     int Nslots, int f_mpool_nofile, int f_rdonly);
#define cspoolbdb_destroy mspoolbdb_destroy
#define cspoolbdb_insert mspoolbdb_insert
#define cspoolbdb_truncate mspoolbdb_truncate
#define cspoolbdb_truncate_oldest mspoolbdb_truncate_oldest
#define cspoolbdb_slots_open mspoolbdb_slots_open
#define cspoolbdb_slots_close mspoolbdb_slots_close
#define cspoolbdb_slots_read mspoolbdb_slots_read
#define cspoolbdb_slots_datasize mspoolbdb_slots_datasize
#define cspoolbdb_set_event_notify mspoolbdb_set_event_notify

int cspoolbdb_slots_open_unlocked(struct mspoolbdb_st *mspool,
				  char *fkey, int *sd);
int cspoolbdb_slots_close_unlocked(struct mspoolbdb_st *mspool, int sd);

#endif

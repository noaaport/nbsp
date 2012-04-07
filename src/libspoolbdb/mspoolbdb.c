/*
 * Copyright (c) 2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include "mspoolbdb.h"

int cspoolbdb_create(struct mspoolbdb_st **mspool,
		     char *dbhome, int dbenv_mode,
		     uint32_t dbcache_mb, uint32_t maxsize_per128,
		     char *dbfname, int dbfile_mode, uint32_t Ndb,
		     int Nslots, int f_mpool_nofile, int f_rdonly){
  int status;
  struct spoolbdbv_st *dbv;
  struct spoolbuf_slots_st *spoolslots;
  struct mspoolbdb_st *ms;

  if(Nslots < 1)
    return(EINVAL);

  ms = malloc(sizeof(struct mspoolbdb_st));
  if(ms == NULL)
    return(errno);

  ms->spoolbdbv = NULL;
  ms->spoolslots = NULL;
  
  /*
   * Passing the next to last argument as 1 creates a (shared) memory
   * (mpool_nofile) db, even if dbhome and dbfile are not NULL.
   */
  status = spoolbdbv_open(&dbv, dbhome, dbenv_mode,
			  dbcache_mb, maxsize_per128, Ndb,
			  dbfname, dbfile_mode, f_mpool_nofile, f_rdonly);
  if(status != 0){
    free(ms);
    return(status);
  }

  spoolslots = spoolbuf_slots_create(Nslots);
  if(spoolslots == NULL){
    (void)spoolbdbv_close(dbv);
    free(ms);
    return(errno);
  }

  ms->spoolbdbv = dbv;
  ms->spoolslots = spoolslots;
  *mspool = ms;

  return(0);
}

int mspoolbdb_create(struct mspoolbdb_st **mspool,
		     char *dbhome, int dbenv_mode,
		     uint32_t dbcache_mb, uint maxsize_per128,
		     uint32_t Ndb, int Nslots){
  int status;

  /*
   * An in-memory db is a special case of the cpsool db in which
   * dbfname is NULL. If dbhome is also NULL then it is a pure in-memory db,
   * otherwise the environment is in the fs. The last argument to
   * cspoolb_create() indicates that the bdb must be created (f_rdonly = 0),
   * and the next to last argument indicates that it is a memory db
   * (f_mpool_nofile = 1).
   */

  status = cspoolbdb_create(mspool, dbhome, dbenv_mode,
			    dbcache_mb, maxsize_per128,
			    NULL, 0, Ndb, Nslots, 1, 0);
  return(status);
}

int mspoolbdb_destroy(struct mspoolbdb_st *mspool){

  int status = 0;

  assert(mspool != NULL);

  if(mspool->spoolslots != NULL)
    spoolbuf_slots_destroy(mspool->spoolslots);

  if(mspool->spoolbdbv != NULL)
    status = spoolbdbv_close(mspool->spoolbdbv);

  free(mspool);

  return(status);
}

int mspoolbdb_insert(struct mspoolbdb_st *mspool,
		     char *fkey, void *fdata, uint32_t fdata_size){
  int status;

  status = spoolbdbv_write2(mspool->spoolbdbv, fkey, fdata, fdata_size);

  return(status);
}

int mspoolbdb_slots_open(struct mspoolbdb_st *mspool, char *fkey, int *sd){

  int status;

  status = spoolbuf_slots_open(mspool->spoolslots, mspool->spoolbdbv, 
			       fkey, sd);
  return(status);
}

int mspoolbdb_slots_close(struct mspoolbdb_st *mspool, int sd){

  int status;

  status = spoolbuf_slots_close(mspool->spoolslots, sd);
 
  return(status);
}

size_t mspoolbdb_slots_read(struct mspoolbdb_st *mspool, int sd,
			    void *data, size_t data_size){
  /*
   * This function is actually unlocked. It should be used only by the
   * thread that opened the slot (the thread that owns sd).
   */
  size_t n;

  n = spoolbuf_slots_read(mspool->spoolslots, sd, data, data_size);

  return(n);
}

size_t mspoolbdb_slots_datasize(struct mspoolbdb_st *mspool, int sd){

  return(spoolbuf_slots_datasize(mspool->spoolslots, sd));
}

int mspoolbdb_set_event_notify(struct mspoolbdb_st *mspool,
    void (*db_event_fcn)(DB_ENV *dbenv, u_int32_t event, void *event_info)){

  int status;

  status = spoolbdbv_set_event_notify(mspool->spoolbdbv, db_event_fcn);

  return(status);
}

/*
 * In the case of the cspool, if only one thread is using the cspool
 * in the client readers, then these functions can be used instead of
 * the locking versions.
 */
int cspoolbdb_slots_open_unlocked(struct mspoolbdb_st *mspool,
				  char *fkey, int *sd){

  int status = 0;

  status = spoolbuf_slots_open_unlocked(mspool->spoolslots, mspool->spoolbdbv, 
					fkey, sd);
  return(status);
}

int cspoolbdb_slots_close_unlocked(struct mspoolbdb_st *mspool, int sd){

  int status = 0;

  status = spoolbuf_slots_close_unlocked(mspool->spoolslots, sd);
 
  return(status);
}

int mspoolbdb_truncate(struct mspoolbdb_st *mspool){

  return(spoolbdbv_truncate(mspool->spoolbdbv));
}

int mspoolbdb_truncate_oldest(struct mspoolbdb_st *mspool){

  return(spoolbdbv_truncate_oldest(mspool->spoolbdbv));
}

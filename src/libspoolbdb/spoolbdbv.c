/*
 * Copyright (c) 2006-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <db.h>
#include "spoolbdb.h"
#include "spoolbdb_priv.h"
#include "spoolbdbv.h"

/*
 * If dbhome == NULL, then it is a pure memory-based private bdb,
 * otherwise the environment is in the fs. Then in that case,
 * if f_mpool_nofile is 0, it is a normal bdb. Otherwise it is a memory
 * based db (but the environment is in the fs). If dbfname is NULL
 * it is private, otherwise it is shared.
 */

#define DBENV_FLAGS_PRIVATE (DB_CREATE | DB_THREAD | DB_INIT_MPOOL | \
	DB_PRIVATE)
#define DBENV_FLAGS_SHARED (DB_CREATE | DB_THREAD | DB_INIT_MPOOL | \
	DB_INIT_CDB)
#define DBENV_FLAGS_RDONLY (DB_THREAD | DB_INIT_MPOOL | DB_INIT_CDB)

static int spoolbdbv_env_open(DB_ENV **dbvenv,
			      char *dbhome, int dbenv_mode,
			      uint32_t dbcache_mb,
			      int f_rdonly);

static int spoolbdbv_env_close(DB_ENV *dbvenv);
static int spoolbdbv_db_open(struct spoolbdbv_st *spoolbdbv,
			     uint32_t Ndb, uint32_t maxsize,
			     DB_ENV *dbenv, char *dbfname, int dbmode,
			     int f_mpool_nofile, int f_rdonly);
static int spoolbdbv_db_close(struct spoolbdbv_st *spoolbdbv);

static int spoolbdbv_env_open(DB_ENV **dbvenv,
			      char *dbhome, int dbenv_mode,
			      uint32_t dbcache_mb,
			      int f_rdonly){
  DB_ENV *dbenv = NULL;
  int status = 0;
  uint32_t cache_size = (dbcache_mb * 1024 * 1024);
  uint32_t dbenv_flags;

  if(dbhome == NULL){
    if(dbenv_mode != 0)
      return(EINVAL);
  }

  status = db_env_create(&dbenv, 0);

  if(status == 0)
    status = dbenv->set_cachesize(dbenv, 0, cache_size, 0);

  if(status == 0){
    if(dbhome == NULL)
      dbenv_flags = DBENV_FLAGS_PRIVATE;
    else if(f_rdonly == 1)
      dbenv_flags = DBENV_FLAGS_RDONLY;
    else
      dbenv_flags = DBENV_FLAGS_SHARED;

    status = dbenv->open(dbenv, dbhome, dbenv_flags, dbenv_mode);
  }

  if(status != 0){
    if(dbenv != NULL)
      (void)dbenv->close(dbenv, 0);
  }else
    *dbvenv = dbenv;

  return(status);
}

static int spoolbdbv_env_close(DB_ENV *dbenv){

  int status = 0;

  status = dbenv->close(dbenv, 0);

  return(status);
}

static int spoolbdbv_db_open(struct spoolbdbv_st *spoolbdbv,
			     uint32_t Ndb, uint32_t maxsize,
			     DB_ENV *dbenv, char *dbfname, int dbmode,
			     int f_mpool_nofile, int f_rdonly){
  /*
   * dbenv cannot be null. Then, if dbfname is NULL it creates an in-memory db.
   * If it is not NULL and f_mpool_nofile is set then it is cache only
   * bdb, accessible by other applications as well (if dbhome was not NULL
   * in the environment).
   *
   * If f_rdonly is set, then underlying bdb's are opened rdonly, by
   * calling spoolbdb_open_rdonly. In this case the values of
   * nmax, dbmode are not used.
   */
  int status = 0;
  unsigned int i;
  char *dbfnamexx = NULL;
  int dbfnamexx_len = 0;

  if(dbenv == NULL)
    return(EINVAL);
  
  spoolbdbv->dbv = calloc(Ndb + 2, sizeof(DB*));
  if(spoolbdbv->dbv == NULL){
    status = errno;
    return(status);
  }

  if(dbfname != NULL){
    dbfnamexx_len = snprintf(NULL, 0, "%s.%u", dbfname, UINT_MAX);
    ++dbfnamexx_len;
    dbfnamexx = malloc(dbfnamexx_len);
    if(dbfnamexx == NULL){
      status = errno;
      goto end;
    }
  }

  for(i = 0; i < Ndb + 2; ++i)
    spoolbdbv->dbv[i] = NULL;

  spoolbdbv->wrdb = NULL;
  spoolbdbv->dbenv = NULL;
  spoolbdbv->Ndb = Ndb;
  spoolbdbv->n = 0;
  spoolbdbv->maxsize = maxsize;
  spoolbdbv->size = 0;

  /*
   * Open all the db's except the first and last, which stay set to NULL
   * as sentinnels.
   */
  for(i = 1; (i <= Ndb) && (status == 0); ++i){
    if(dbfname != NULL){
      snprintf(dbfnamexx, dbfnamexx_len, "%s.%u", dbfname, i);
    }
    status = spoolbdb_open(&spoolbdbv->dbv[i], dbenv,
			   dbfnamexx, dbmode, f_mpool_nofile, f_rdonly);
  }

  if(status == 0){
    spoolbdbv->wrdb = &spoolbdbv->dbv[1];
    spoolbdbv->dbenv = dbenv;
  }

 end:

  if(dbfnamexx != NULL)
    free(dbfnamexx);

  if(status != 0)
    (void)spoolbdbv_db_close(spoolbdbv);

  return(status);
}

static int spoolbdbv_db_close(struct spoolbdbv_st *spoolbdbv){

  uint32_t i;
  int status = 0, status1 = 0;

  for(i = 1; i <= spoolbdbv->Ndb; ++i){
    if(spoolbdbv->dbv[i] != NULL){
      status1 = spoolbdb_close(spoolbdbv->dbv[i]);
      if(status == 0)
	status = status1;
    }
  }

  free(spoolbdbv->dbv);

  return(status);
}

int spoolbdbv_open(struct spoolbdbv_st **spoolbdbv,
		   char *dbhome, int dbenv_mode,
		   uint32_t dbcache_mb, uint32_t maxsize_per128,
		   uint32_t Ndb, char *dbfname, int dbfile_mode,
		   int f_mpool_nofile, int f_rdonly){ 
  /*
   * If f_rdonly is set, then the values of
   *
   *     dbenv_mode, nmax, dbfile_mode
   *
   * are irrelevant. Here dbcache_mb is the size for each db. The total size
   * that is passed to the environment is dbcache_mb * Ndb. Each db is
   * not allowed to grow larger than 96/128 (75%) of the dbcache_mb.
   */
  int status = 0;
  DB_ENV *dbenv = NULL;
  struct spoolbdbv_st *sdbv = NULL;
  uint32_t cache_size = (dbcache_mb * 1024 * 1024);
  uint32_t maxsize;	/* for each db */

  assert(dbcache_mb >= 1);
  assert(Ndb >= 1);
  assert(maxsize_per128 <= 128);

  if((dbcache_mb == 0) || (Ndb == 0) || (maxsize_per128 > 128))
    return(EINVAL);

  maxsize = (cache_size/128) * maxsize_per128;

  if(dbhome == NULL){
    /*
     * This is an in-memory (private) db.
     */
    if((dbfname != NULL) || (f_mpool_nofile == 0))
      return(EINVAL);
  }else{
    /*
     * If f_mpool_nofile is 0 this a normal bdb. Otherwise it is a memory
     * based db, but the environment is in the fs. If dbfname is NULL
     * it is private, otherwise it is shared.
     */
      ;
  }

  sdbv = malloc(sizeof(struct spoolbdbv_st));
  if(sdbv == NULL){
    status = errno;
    return(status);
  }

  status = pthread_mutex_init(&sdbv->mutex, NULL);
  if(status != 0){
    free(sdbv);
    return(status);
  }

  /*
   * Create and/or open according to the values of dbhome and f_rdonly.
   * The dbcache_mb is specified for each db.
   */
  status = spoolbdbv_env_open(&dbenv, dbhome, dbenv_mode, dbcache_mb * Ndb,
			      f_rdonly);
  if(status != 0){
    pthread_mutex_destroy(&sdbv->mutex);
    free(sdbv);
    return(status);
  }

  status = spoolbdbv_db_open(sdbv, Ndb, maxsize,
			     dbenv, dbfname, dbfile_mode,
			     f_mpool_nofile, f_rdonly);
  if(status != 0){
    if(dbenv != NULL)
      (void)spoolbdbv_env_close(dbenv);

    pthread_mutex_destroy(&sdbv->mutex);
    free(sdbv);
  }else
    *spoolbdbv = sdbv;

  return(status);
}

int spoolbdbv_close(struct spoolbdbv_st *spoolbdbv){

  int status = 0, status1 = 0;

  status = spoolbdbv_db_close(spoolbdbv);

  if(spoolbdbv->dbenv != NULL){
    status1 = spoolbdbv_env_close(spoolbdbv->dbenv);
    spoolbdbv->dbenv = NULL;
    if(status == 0)
      status = status1;
  }

  pthread_mutex_destroy(&spoolbdbv->mutex);

  free(spoolbdbv);

  return(status);
}

int spoolbdbv_truncate(struct spoolbdbv_st *spoolbdbv){
  /*
   * This function truncates all db's. It is meant to be used as a way
   * out of emergency situations such as the bdb system encountering
   * memory error conditions (for example, if not enough cache memory
   * was reserved).
   */
  int status = 0;
  uint32_t count;
  int i;
  DB *dbp;

  status = pthread_mutex_lock(&spoolbdbv->mutex);
  if(status != 0)
    return(status);

  for(i = spoolbdbv->Ndb; (i >= 1) && (status == 0); --i){
    dbp = spoolbdbv->dbv[i];
    status = dbp->truncate(dbp, NULL, &count, 0);
    if(status == 0){
      spoolbdbv->wrdb = &spoolbdbv->dbv[i];
      spoolbdbv->n = 0;
      spoolbdbv->size = 0;
      /*
       * log_info("XXX 2 truncated %d", i);
       */
    }
  }

  pthread_mutex_unlock(&spoolbdbv->mutex);

  return(status);
}

int spoolbdbv_truncate_oldest(struct spoolbdbv_st *spoolbdbv){
  /*
   * This function is similar in spirit to the above, but it only
   * truncates the oldest db, with the intention of releasing some
   * memory.
   */
  int status = 0;
  uint32_t count;
  DB **dbpv;

  status = pthread_mutex_lock(&spoolbdbv->mutex);
  if(status != 0)
    return(status);

  dbpv = spoolbdbv->wrdb;
  ++dbpv;
  if(*dbpv == NULL)
    dbpv = &spoolbdbv->dbv[1];

  status = (*dbpv)->truncate(*dbpv, NULL, &count, 0);
  if(status == 0){
    spoolbdbv->wrdb = dbpv;
    spoolbdbv->n = 0;
    spoolbdbv->size = 0;
    /* 
     * log_info("XXX 1 truncated oldest");
     */
  }

  pthread_mutex_unlock(&spoolbdbv->mutex);

  return(status);
}

int spoolbdbv_read(struct spoolbdbv_st *spoolbdbv,
		  char *fkey, struct spoolbuf_st *spoolb){

  int status = 0;

  status = pthread_mutex_lock(&spoolbdbv->mutex);
  if(status != 0)
    return(status);

  status = spoolbdbv_read_unlocked(spoolbdbv, fkey, spoolb);

  pthread_mutex_unlock(&spoolbdbv->mutex);

  return(status);
}

int spoolbdbv_write(struct spoolbdbv_st *spoolbdbv,
		   char *fkey, struct spoolbuf_st *spoolb){

  int status = 0;

  status = pthread_mutex_lock(&spoolbdbv->mutex);
  if(status != 0)
    return(status);

  status = spoolbdbv_write_unlocked(spoolbdbv, fkey, spoolb);

  pthread_mutex_unlock(&spoolbdbv->mutex);

  return(status);
}

int spoolbdbv_write2(struct spoolbdbv_st *spoolbdbv,
		    char *fkey, void *data, uint32_t datasize){

  int status = 0;

  status = pthread_mutex_lock(&spoolbdbv->mutex);
  if(status != 0)
    return(status);

  status = spoolbdbv_write2_unlocked(spoolbdbv, fkey, data, datasize);

  pthread_mutex_unlock(&spoolbdbv->mutex);

  return(status);
}

int spoolbdbv_read_unlocked(struct spoolbdbv_st *spoolbdbv,
			   char *fkey, struct spoolbuf_st *spoolb){
  int status = 0;
  DB **dbpv = spoolbdbv->wrdb;

  status = spoolbdb_read(*dbpv, fkey, spoolb);
  while(status == DB_NOTFOUND){
    --dbpv;
    if(*dbpv == NULL)
      dbpv = &spoolbdbv->dbv[spoolbdbv->Ndb];

    if(dbpv == spoolbdbv->wrdb)
      break;

    status = spoolbdb_read(*dbpv, fkey, spoolb);
  }

  return(status);
}
  
int spoolbdbv_write_unlocked(struct spoolbdbv_st *spoolbdbv,
			    char *fkey, struct spoolbuf_st *spoolb){

  int status = 0;

  status = spoolbdbv_write2_unlocked(spoolbdbv, fkey,
				    spoolb->buffer, spoolb->buffer_size);
  return(status);
}

int spoolbdbv_write2_unlocked(struct spoolbdbv_st *spoolbdbv,
			     char *fkey, void *data, uint32_t datasize){
  int status = 0;
  uint32_t count;

  /*
   * If the data does not fit in one db there is nothing to do but
   * report it and let the application drop it eventually.
   */
  if(datasize > spoolbdbv->maxsize)
    return(ENOMEM);

  /*
   * If the data fits in the remaining space of the current db it is
   * inserted. Otherwise the oldest db is truncated and then it is inserted
   * there. It is possible that the data in principle fits in the current db
   * (i.e. this if is not entered) but that the write2 function below
   * returns ENOMEM from the bdb library.
   * Therefore, the application has to check for this (or any other) error
   * anyway. However, if the application truncates the oldest db and calls
   * again this function, then provided that the data fits in one db
   * (as checked above) the write2 below should succeed.
   */
  if(spoolbdbv->size + datasize > spoolbdbv->maxsize){
    ++spoolbdbv->wrdb;
    if(*spoolbdbv->wrdb == NULL)
      spoolbdbv->wrdb = &spoolbdbv->dbv[1];

    status = (*spoolbdbv->wrdb)->truncate(*spoolbdbv->wrdb, NULL, &count, 0);
    if(status == 0){
      spoolbdbv->n = 0;
      spoolbdbv->size = 0;
    }
  }

  if(status == 0)
    status = spoolbdb_write2(*spoolbdbv->wrdb, fkey, data, datasize);

  if(status == 0){
    /*
     * log_info("XXXX spoolbdbv_write2_unlocked: Inserted %s in slot %u",
     *		fkey, spoolbdbv->n);
     */
    ++spoolbdbv->n;
    spoolbdbv->size += datasize;
  }

  return(status);
}

int spoolbdbv_set_event_notify(struct spoolbdbv_st *spoolbdbv,
    void (*db_event_fcn)(DB_ENV *dbenv, u_int32_t event, void *event_info)){

  int status;

  status = (spoolbdbv->dbenv)->set_event_notify(spoolbdbv->dbenv,
						db_event_fcn);

  return(status);
}

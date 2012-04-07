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
#include "spoolbdb.h"
#include "spoolbdb_priv.h"

/*
 * Strategy:
 *
 * dbenv cannot be NULL
 * dbfname == NULL => an in-memory (private) bdb.
 * dbfname != NULL => look at f_mpool_nofile.
 * f_mpool_nofile == 1 => shared memory spool. The dbfname is
 *                        used as the db name (not the file).
 * f_mpool_nofile == 0 => normal (file backed) db
 */

#define SPOOLBDB_FLAGS_CREATE (DB_THREAD | DB_CREATE)
#define SPOOLBDB_FLAGS_RDONLY (DB_THREAD | DB_RDONLY)

static int spoolbdb_open_create(DB **db, DB_ENV *dbenv,
				char *dbfname, int dbmode,
				int f_mpool_nofile);
static int spoolbdb_open_rdonly(DB **db, DB_ENV *dbenv, char *dbfname,
				int f_mpool_nofile);

#if 0
#include "../err.h"
static void XXX_db_errcall_fcn(const DB_ENV *dbenv,
			       const char *errpfx, const char *msg){
  log_info("XXX %s", msg);
}
#endif

static int spoolbdb_open_create(DB **db, DB_ENV *dbenv,
				char *dbfname, int dbmode,
				int f_mpool_nofile){
  /*
   * If dbfname == NULL, then it is an in-memory (private) bdb.
   * If dbfname != NULL, then if the flag f_mpool_nofile is 1 the db
   * is configured as shared memory db or a normal db if the flag is 0.
   */
  DB *dbp = NULL;
  DB_MPOOLFILE *mpf = NULL;
  int status = 0;
  uint32_t dbflags = SPOOLBDB_FLAGS_CREATE;

  if(dbenv == NULL)
    return(EINVAL);

  /*
   * Let the user know that this combination is not valid.
   */
  if((dbfname == NULL) && (f_mpool_nofile == 0))
    return(EINVAL);

  status = db_create(&dbp, dbenv, 0);
  if(status != 0)
    return(status);

  if((dbfname == NULL) || (f_mpool_nofile == 1)){
    /*
     * Private or shared memory db.
     */
    mpf = dbp->get_mpf(dbp);
    status = mpf->set_flags(mpf, DB_MPOOL_NOFILE, 1);
  }

  if(status == 0){
    if(dbfname == NULL)
      status = dbp->open(dbp, NULL, NULL, NULL, DB_BTREE, dbflags, dbmode);
    else if(f_mpool_nofile != 0)
      status = dbp->open(dbp, NULL, NULL, dbfname, DB_BTREE, dbflags, dbmode);
    else
      status = dbp->open(dbp, NULL, dbfname, NULL, DB_BTREE, dbflags, dbmode);
  }

#if 0
  dbp->set_errcall(dbp, XXX_db_errcall_fcn); /* XXX */
#endif

  if(status != 0){
    if(dbp != NULL)
    (void)dbp->close(dbp, 0);
  }else
    *db = dbp;

  return(status);
}

static int spoolbdb_open_rdonly(DB **db, DB_ENV *dbenv, char *dbfname,
				int f_mpool_nofile){
  /*
   * If f_mpool_nofile is set, we are opening a shared memory db, otherwise
   * it is a normal db.
   */
  DB *dbp = NULL;
  int status = 0;
  uint32_t dbflags = SPOOLBDB_FLAGS_RDONLY;

  if((dbenv == NULL) || (dbfname == NULL))
      return(EINVAL);

  status = db_create(&dbp, dbenv, 0);
  if(status != 0)
    return(status);

  if(f_mpool_nofile != 0)
    status = dbp->open(dbp, NULL, NULL, dbfname, DB_BTREE, dbflags, 0);
  else
    status = dbp->open(dbp, NULL, dbfname, NULL, DB_BTREE, dbflags, 0);

  if(status != 0){
    if(dbp != NULL)
    (void)dbp->close(dbp, 0);
  }else
    *db = dbp;

  return(status);
}

int spoolbdb_open(DB **db, DB_ENV *dbenv,
		  char *dbfname, int dbmode,
		  int f_mpool_nofile, int f_rdonly){
  /*
   * If f_rdonly is set, then the argument dbmode is not used.
   */
  int status;

  if(f_rdonly == 1)
    status = spoolbdb_open_rdonly(db, dbenv, dbfname, f_mpool_nofile);
  else
    status = spoolbdb_open_create(db, dbenv, dbfname, dbmode, f_mpool_nofile);

  return(status);
}

int spoolbdb_close(DB *dbp){

  int status = 0;

  if(dbp != NULL)
    status = dbp->close(dbp, 0);

  return(status);
}

int spoolbdb_write(DB *dbp, char *fkey, struct spoolbuf_st *spoolb){

  int status;

  status = spoolbdb_write2(dbp, fkey, spoolb->buffer, spoolb->buffer_size);

  return(status);
}

int spoolbdb_write2(DB *dbp, char *fkey, void *buffer, uint32_t buffersize){

  int status;
  DBT key, data;

  memset(&key, 0 , sizeof(DBT));
  memset(&data, 0 , sizeof(DBT));

  key.data = fkey;
  key.size = strlen(fkey);
  data.data = buffer;
  data.size = buffersize;

  status = dbp->put(dbp, NULL, &key, &data, 0);

  return(status);
}

int spoolbdb_read(DB *dbp, char *fkey, struct spoolbuf_st *spoolb){

  int status;
  void *p;
  DBT key, data;

  memset(&key, 0 , sizeof(DBT));
  memset(&data, 0 , sizeof(DBT));

  key.data = fkey;
  key.size = strlen(fkey);
  data.data = spoolb->buffer;
  data.ulen = spoolb->max_buffer_size;
  data.flags = DB_DBT_USERMEM;

  status = dbp->get(dbp, NULL, &key, &data, 0);
  if(status == DB_BUFFER_SMALL){
    p = malloc(data.size);
    if(p != NULL){
      if(spoolb->buffer != NULL)
	free(spoolb->buffer);

      spoolb->buffer = p;
      spoolb->max_buffer_size = data.size;
      data.data = spoolb->buffer;
      data.ulen = spoolb->max_buffer_size;
      status = dbp->get(dbp, NULL, &key, &data, 0);
    }else
      status = errno;
  }

  if(status == 0){
    spoolb->buffer_size = data.size;
    spoolb->readp = (char*)spoolb->buffer;
    spoolb->nread = 0;
  }

  return(status);
}

struct spoolbuf_st *spoolbuf_create(void){

  struct spoolbuf_st *spoolb = NULL;

  spoolb = malloc(sizeof(struct spoolbuf_st));
  if(spoolb == NULL)
    return(NULL);

  spoolb->buffer = NULL;
  spoolb->buffer_size = 0;
  spoolb->max_buffer_size = 0;
  spoolb->readp = (char*)spoolb->buffer;
  spoolb->nread = 0;

  return(spoolb);
}

void spoolbuf_destroy(struct spoolbuf_st *spoolb){

  assert(spoolb != NULL);

  if(spoolb->buffer != NULL)
    free(spoolb->buffer);

  free(spoolb);
}

void *spoolbuf_data(struct spoolbuf_st *spoolb){

  return(spoolb->buffer);
}

size_t spoolbuf_datasize(struct spoolbuf_st *spoolb){

  return((size_t)spoolb->buffer_size);
}

size_t spoolbuf_maxsize(struct spoolbuf_st *spoolb){

  return((size_t)spoolb->max_buffer_size);
}

size_t spoolbuf_read(struct spoolbuf_st *spoolb, void *data, size_t size){

  size_t n = size;

  if(spoolb->nread == spoolb->buffer_size)
    return(0);

  if(spoolb->nread + n > spoolb->buffer_size)
    n = spoolb->buffer_size - spoolb->nread;

  memcpy(data, spoolb->readp, n);
  spoolb->nread += n;
  spoolb->readp += n;

  return(n);
}
  
void spoolbuf_read_init(struct spoolbuf_st *spoolb){

  spoolb->readp = (char*)spoolb->buffer;
  spoolb->nread = 0;
}

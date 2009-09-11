/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "pctl.h"

/*
 * These functions are used in pctldb.c only.
 *
 * The pctldb stores the pce's in a queue db. The mf of each pce
 * is stored in a btree db, indexed by the struct timespec of the product.
 */

#define PCTLMFDB_FLAGS	(DB_CREATE | DB_THREAD)

static int timespec_cmp(DB *dbp, const DBT *a, const DBT *b);

int pctlmfdb_open(struct pctldb_st *pctldb,
		  struct pctldb_param_st *param, int *dberror){

  DB *mfdbp = NULL;
  int status;
  uint32_t dbflags = PCTLMFDB_FLAGS;

  status = db_create(&mfdbp, param->dbenv, 0);

  if(status == 0)
    status = mfdbp->set_bt_compare(mfdbp, timespec_cmp);

  if(status == 0)
    status = mfdbp->open(mfdbp, NULL, param->mfdbname, NULL, DB_BTREE, dbflags,
			 param->mode);

  if(status != 0){
    if(mfdbp != NULL)
      mfdbp->close(mfdbp, 0);

    *dberror = status;
    status = -1;
  }

  if(status == 0)
    pctldb->mfdbp = mfdbp;

  return(status);
}

int pctlmfdb_close(struct pctldb_st *pctldb, int *dberror){

  int status = 0;
  DB *mfdbp = NULL;

  assert(pctldb != NULL);

  mfdbp = pctldb->mfdbp;    
  if(mfdbp != NULL)
    status = mfdbp->close(mfdbp, 0);

  if(status != 0){
    *dberror = status;
    status = -1;
  }

  return(status);
}

int pctlmfdb_snd(struct pctldb_st *pctldb, struct timespec *ts,
		 struct memfile_st *mf, int *dberror){

  int status;
  DBT key, data;
  DB* mfdbp = pctldb->mfdbp;

  memset(&key, 0 , sizeof(DBT));
  memset(&data, 0 , sizeof(DBT));

  key.data = ts;
  key.size = sizeof(struct timespec);
  data.data = mf->p;
  data.size = get_memfile_data_size(mf); /* we need only the used portion */

  if((status = mfdbp->put(mfdbp, NULL, &key, &data, 0)) != 0){
    *dberror = status;
    status = -1;
  }else
    pctldb->mf_total_size += data.size;

  return(status);
}

int pctlmfdb_rcv(struct pctldb_st *pctldb, struct timespec *ts,
		 struct memfile_st *mf, int *dberror){

  int status = 0;
  int dberror_del;
  DBT key, data;
  DB* mfdbp = pctldb->mfdbp;

  memset(&key, 0 , sizeof(DBT));
  memset(&data, 0 , sizeof(DBT));

  key.data = ts;
  key.ulen = sizeof(struct timespec);
  key.flags = DB_DBT_USERMEM;
  data.data = mf->p;
  data.ulen = get_memfile_allocated_size(mf);
  data.flags = DB_DBT_USERMEM;
  *dberror = mfdbp->get(mfdbp, NULL, &key, &data, 0);
  if(*dberror == DB_BUFFER_SMALL){
    status = realloc_memfile(mf, data.size);
    if(status == 0){
      data.data = mf->p;
      data.ulen = get_memfile_allocated_size(mf);
      *dberror = mfdbp->get(mfdbp, NULL, &key, &data, 0);
    }else
      *dberror = errno;
  }

  if(*dberror == 0)
    mf->size = data.size;
  else{
    status = -1;
    mf->size = 0; /* Only for convenience in the substraction below */
  }

  dberror_del = mfdbp->del(mfdbp, NULL, &key, 0);

  if(dberror_del == 0){
    if(pctldb->mf_total_size > mf->size)
      pctldb->mf_total_size -= mf->size;
    else
      pctldb->mf_total_size = 0;
  }else{
    if(*dberror == 0){
      status = -1;
      *dberror = dberror_del;
    }
  }

  return(status);
}

static int timespec_cmp(DB *dbp __attribute__((unused)),
			const DBT *a, const DBT *b){

  struct timespec ts1, ts2;
  int size;

  size = sizeof(struct timespec);
  memcpy(&ts1, a->data, size);
  memcpy(&ts2, b->data, size);

  if(ts1.tv_sec != ts2.tv_sec)
    return(ts1.tv_sec < ts2.tv_sec ? -1 : 1);

  if(ts1.tv_nsec != ts2.tv_nsec)
    return(ts1.tv_nsec < ts2.tv_nsec ? -1 : 1);

  return(0);
}

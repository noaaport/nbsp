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
#include <sys/time.h>   /* gettimeofday */
#include <errno.h>
#include "oscompat.h"
#include "pctl.h"

#define PCTLDB_FLAGS	(DB_CREATE | DB_THREAD)

static int pctldb_stat(struct pctldb_st *pctldb);
static int pctlmfdb_stat(struct pctldb_st *pctldb, int *dberror);
static int pctldb_soft_limit(struct pctldb_st *pctldb);
static int pctldb_hard_limit(struct pctldb_st *pctldb, uint32_t mf_size);
static void pctldb_set_timeout(unsigned int timeout_ms,
			       struct timespec *timeout);

int pctldb_open(struct pctldb_st **pctldb,
		struct pctldb_param_st *param, int *dberror){

  DB *dbp = NULL;
  int status;
  int dberror_close;
  uint32_t dbflags = PCTLDB_FLAGS;
  struct pctldb_st *p = NULL;

  status = db_create(&dbp, param->dbenv, 0);
  if(status == 0)
    status = dbp->set_re_len(dbp, param->reclen);

  if(status == 0){
    /* 
     * For a memory based queue (indicated by param->dbname set to NULL)
     * setting the extent size produces an error.
     */
    if((param->extent_size > 0) && (param->dbname != NULL))
      status = dbp->set_q_extentsize(dbp, param->extent_size);
  }

  if(status == 0)
    status = dbp->open(dbp, NULL, param->dbname, NULL, DB_QUEUE, dbflags,
		       param->mode);

  if(status == 0){
    p = malloc(sizeof(struct pctldb_st));
    if(p == NULL)
      status = errno;
  }

  if(status == 0){
    p->dbp = dbp;
    p->mfdbp = NULL;
    p->pce_size = const_pce_size();
    p->pce_data_size = param->reclen;
    p->n = 0;				/* filled by pctldb_stat() below */
    p->nmax = 0xffffffff;
    p->mf_total_size = 0;		/* filled below after pctlmf open */
    p->last_tsqueue.tv_sec = 0;
    p->last_tsqueue.tv_nsec = 0;
    p->softlimit = param->softlimit;
    p->hardlimit = param->hardlimit;
    p->mf_softlimit_mb = param->mf_softlimit_mb;
    p->mf_hardlimit_mb = param->mf_hardlimit_mb;
    status = pctldb_stat(p);
  }

  if(status == 0){
    status = pthread_mutex_init(&p->mutex, NULL);
    if(status == 0){
      status = pthread_cond_init(&p->cond, NULL);
      if(status != 0)
	pthread_mutex_destroy(&p->mutex);
    }
  }

  if(status != 0){
    *dberror = status;
    status = -1;
  }

  /* The mf db */
  if(status == 0)
    status = pctlmfdb_open(p, param, dberror);

  if(status == 0)
    status = pctlmfdb_stat(p, dberror);

  if(status != 0){
    if(p->mfdbp != NULL)
      pctlmfdb_close(p, &dberror_close);	/* error ignored here if any */

    if(dbp != NULL)
      dbp->close(dbp, 0);

    if(p != NULL)
      free(p);

    *dberror = status;
    status = -1;
  }

  if(status == 0)
    *pctldb = p;

  return(status);
}

int pctldb_close(struct pctldb_st *pctldb, int *dberror){

  int status = 0;
  int status1 = 0;
  int dberror1;
  DB *dbp = NULL;

  assert(pctldb != NULL);

  pthread_mutex_lock(&pctldb->mutex);

  dbp = pctldb->dbp;
  if(dbp != NULL){
    if((*dberror = dbp->close(dbp, 0)) != 0)
      status = -1;
  }

  status1 = pctlmfdb_close(pctldb, &dberror1);
  if(status == 0){
    status = status1;
    *dberror = dberror1;
  }

  pthread_mutex_unlock(&pctldb->mutex);
  pthread_mutex_destroy(&pctldb->mutex);
  pthread_cond_destroy(&pctldb->cond);

  free(pctldb);

  return(status);
}

int pctldb_snd(struct pctldb_st *pctldb, void *p, uint32_t size, int *dberror){

  int status;
  DBT key, data;
  uint32_t recno;
  DB* dbp = pctldb->dbp;

  memset(&key, 0 , sizeof(DBT));
  memset(&data, 0 , sizeof(DBT));

  key.data = &recno;
  key.ulen = sizeof(recno);
  key.flags = DB_DBT_USERMEM;
  data.data = p;
  data.size = size;

  if((status = dbp->put(dbp, NULL, &key, &data, DB_APPEND)) == 0)
    ++pctldb->n;
  else{
    *dberror = status;
    status = -1;
  }

  return(status);
}

int pctldb_rcv(struct pctldb_st *pctldb, void *p, uint32_t *size, int *dberror){
  int status = 0;
  DBT key, data;
  uint32_t recno;
  uint32_t reclen;
  DB* dbp = pctldb->dbp;

  assert(p != NULL);
  if(p == NULL){
    *dberror = EINVAL;
    return(-1);
  }    
  
  memset(&key, 0 , sizeof(DBT));
  memset(&data, 0 , sizeof(DBT));

  if((status = dbp->get_re_len(dbp, &reclen)) != 0){
    *dberror = status;
    return(-1);
  }

  if(*size < reclen){
    *dberror = EINVAL;
    return(-1);
  }

  key.data = &recno;
  key.ulen = sizeof(recno);
  key.flags = DB_DBT_USERMEM;
  data.data = p;
  data.ulen = reclen;
  data.flags = DB_DBT_USERMEM;

  status = dbp->get(dbp, NULL, &key, &data, DB_CONSUME);
  if(status != 0){
    *dberror = status;
    status = -1;
  }else{
    *size = reclen;
    --pctldb->n;
  }

  return(status);
}

static int pctldb_stat(struct pctldb_st *pctldb){

  int status;
  DB_QUEUE_STAT *qs = NULL;
  DB* dbp = pctldb->dbp;

  if((status = dbp->stat(dbp, NULL, &qs, 0)) == 0){
    pctldb->n = qs->qs_ndata;
  }

  if(qs != NULL)
    free(qs);

  return(status);
}

static int pctlmfdb_stat(struct pctldb_st *pctldb, int *dberror){

  int status = 0;
  DB* mfdbp = pctldb->mfdbp;
  DBC *cursor;
  DBT key, data;
  uint32_t memfile_size;

  *dberror = mfdbp->cursor(mfdbp, NULL, &cursor, 0);
  if(*dberror != 0)
    return(-1);

  memset(&key, 0, sizeof(DBT));
  memset(&data, 0, sizeof(DBT));

  pctldb->mf_total_size = 0;
  while(status == 0){
    status = cursor->c_get(cursor, &key, &data, DB_NEXT);
    if(status == 0){
      memfile_size = data.size;
      pctldb->mf_total_size += memfile_size;
    }
  }

  cursor->c_close(cursor);

  if(status == DB_NOTFOUND)
    status = 0;
  else{
    *dberror = status;
    status = -1;
  }

  return(status);
}

int pctldb_send_pce(struct pctldb_st *pctldb,
		    struct pctl_element_st *pce, int *dberror){
  /*
   * Returns:
   * 
   * 0 no error
   * 1 soft limit reached
   * 2 hard limit reached
   * -1 real error
   */
  int status = 0;
  uint32_t mf_data_size;
  /* uint32_t n; */

  status = pthread_mutex_lock(&pctldb->mutex);
  if(status != 0){
    *dberror = status;
    return(-1);
  }

  if((status = oscompat_clock_gettime(&pce->tsqueue)) != 0){
    assert(status == 0);
    status = 0;
    pce->tsqueue.tv_sec = time(NULL);
    pce->tsqueue.tv_nsec = 0;
  }

  /*
   * If the clock resolution is not enough to separate this product from
   * the last one, we resolve it manually. This is required in cygwin,
   * where the resolution is only ms. Perhaps the best is to
   * use a sequence number instead of the tsqueue.
   */
  if(pce->tsqueue.tv_sec == pctldb->last_tsqueue.tv_sec){
    if(pce->tsqueue.tv_nsec <= pctldb->last_tsqueue.tv_nsec)
      pce->tsqueue.tv_nsec = pctldb->last_tsqueue.tv_nsec + 1;
  }
  memcpy(&pctldb->last_tsqueue, &pce->tsqueue, sizeof(struct timespec));
  
  /* n = pctldb->n; */

  mf_data_size = get_memfile_data_size(pce->mf);
  if(pctldb_hard_limit(pctldb, mf_data_size) != 0)
     status = 2;

  if(status == 0)
    status = pctldb_snd(pctldb, pce, pctldb->pce_data_size, dberror);

  if(status == 0)
    status = pctlmfdb_snd(pctldb, &pce->tsqueue, pce->mf, dberror);

  if(status == 0){
    if(pctldb_soft_limit(pctldb) != 0)
      status = 1;
  }

  pthread_cond_signal(&pctldb->cond);
  pthread_mutex_unlock(&pctldb->mutex);

  return(status);
}

int pctldb_rcv_pce(struct pctldb_st *pctldb,
		   struct pctl_element_st *pce,
		   int timeout_ms,
		   int *dberror){
  /*
   * The pce here should be the output pce of the pctl, with its
   * pce->mf allocated. 
   *
   * Returns:
   *
   *  0 if there are no errors reading.
   * -1 if there were errors
   *  1 if the queue was empty (after the timeout)
   */
  int status = 0;
  struct timespec timeout;
  uint32_t size;
  struct memfile_st *mf;

  status = pthread_mutex_lock(&pctldb->mutex);
  if(status != 0){
    *dberror = status;
    return(-1);
  }

  if(timeout_ms >= 0)
    pctldb_set_timeout((unsigned int)timeout_ms, &timeout);

  while((pctldb->n == 0) && (status == 0)){
    if(timeout_ms >= 0)
      status = pthread_cond_timedwait(&pctldb->cond, &pctldb->mutex, &timeout);
    else
      status = pthread_cond_wait(&pctldb->cond, &pctldb->mutex);
  }

  if(pctldb->n == 0){
    if(status == ETIMEDOUT)
      status = 1;
    else{
      *dberror = status;
      status = -1;
    }
  }else{
    /*
     * When copying the pce data from the db, it will overwrite the pce->mf
     * of the output pce with a non-valid value (whatever was saved
     * from the active channel pce). So it must saved and restored after
     * retrieveing the mf from mf db.
     */
    size = pctldb->pce_data_size;
    mf = pce->mf;		/* save the correct mf of the output pce */
    status = pctldb_rcv(pctldb, pce, &size, dberror);
    if(status == 0){
      status = pctlmfdb_rcv(pctldb, &pce->tsqueue, mf, dberror);
    }
    pce->mf = mf;		/* restore it */
  }
  pthread_mutex_unlock(&pctldb->mutex);

  return(status);
}

int pctldb_sync(struct pctldb_st *pctldb, int *dberror){

  int status = 0;
  DB* mfdbp = pctldb->mfdbp;

  status = pthread_mutex_lock(&pctldb->mutex);
  if(status != 0){
    *dberror = status;
    return(-1);
  }
    
  if((*dberror = mfdbp->sync(mfdbp, 0)) != 0)
    status = -1;

  pthread_mutex_unlock(&pctldb->mutex);
 
  return(status);
}

static int pctldb_hard_limit(struct pctldb_st *pctldb, uint32_t mf_size){

  int status = 0;
  size_t mb = 1024 * 1024;
  size_t newsize;

  newsize = pctldb->mf_total_size + mf_size;

  if(pctldb->mf_hardlimit_mb > 0){
    if(newsize > pctldb->mf_hardlimit_mb * mb)
      return(1);
  }

  if(pctldb->hardlimit > 0){
    if(pctldb->n == pctldb->hardlimit)
      return(1);
  }

  return(status);
}

static int pctldb_soft_limit(struct pctldb_st *pctldb){

  int status = 0;
  size_t mb = 1024 * 1024;
  size_t newsize;

  newsize = pctldb->mf_total_size;

  if(pctldb->mf_softlimit_mb > 0){
    if(newsize > pctldb->mf_softlimit_mb * mb)
      return(1);
  }

  if(pctldb->softlimit > 0){
    if(pctldb->n >= pctldb->softlimit)
      return(1);
  }

  return(status);
}

/*
 * Utility function to set the default pthreads timeout
 */
static void pctldb_set_timeout(unsigned int timeout_ms,
			       struct timespec *timeout){

  struct timeval tv;

  gettimeofday(&tv, NULL);
  timeout->tv_sec = tv.tv_sec;
  timeout->tv_nsec = tv.tv_usec * 1000;

  timeout->tv_sec += (timeout_ms/1000);
  timeout->tv_nsec += (timeout_ms % 1000)*1000000;
}

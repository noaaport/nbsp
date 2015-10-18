/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
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
#include "qdb.h"

#define QDB_FLAGS	(DB_CREATE | DB_THREAD)

static int qdb_stat(struct nbspq_st *nbspq);
static int queue_soft_limit(int n, int softlimit);
static int queue_hard_limit(int n, int hardlimit);
static void queue_set_timeout(unsigned int timeout_ms,
			      struct timespec *timeout);

nbspqtable_t *nbspqt_open(u_int8_t num_queues, int softlimit, int hardlimit,
			  struct qdb_param_st *qdbparam, int *dberror){

  int status = 0;
  int dberror_close = 0;
  struct nbsp_qtable_st *qt = NULL;
  char *dbfname = NULL;
  int dbfname_size = 0;
  struct qdb_param_st qdbparam_copy;
  int size;
  int i;
  int memory_based_db = 0;

  qt = malloc(sizeof(struct nbsp_qtable_st));
  if(qt == NULL){
    *dberror = errno;
    return(NULL);
  }

  size = num_queues * sizeof(struct nbspq_st*);
  qt->nbspq = malloc(size);
  if(qt->nbspq == NULL){
    free(qt);
    *dberror = errno;
    return(NULL);
  }

  qt->n = 0;
  qt->softlimit = softlimit;
  qt->hardlimit = hardlimit;
  qt->reclen = qdbparam->reclen;
  qt->appdata = NULL;

  /* If the db file name is NULL or "" it is a memory based queue. */
  if((qdbparam->dbname == NULL) || (qdbparam->dbname[0] == '\0'))
    memory_based_db = 1;

  if(memory_based_db == 0){
    /* leave room to add one byte in hex + ".db" + '\0' */
    dbfname_size = strlen(qdbparam->dbname) + 6;	
    dbfname = malloc(dbfname_size);
    if(dbfname == NULL){
      free(qt->nbspq);
      free(qt);
      *dberror = errno;
      return(NULL);
    }
  }

  memcpy(&qdbparam_copy, qdbparam, sizeof(qdbparam_copy));
  for(i = 0; (i <= num_queues - 1) && (status == 0); ++i){
    if(memory_based_db == 0){
      snprintf(dbfname, dbfname_size, "%s%x.db", qdbparam->dbname, i);
      qdbparam_copy.dbname = dbfname;
    }
    status = qdb_open(&(qt->nbspq[i]), &qdbparam_copy, dberror);
    if(status == 0)
      ++qt->n;
    else
      break;
  }
  if(dbfname != NULL)
    free(dbfname);

  if(status != 0){
    for(i = 0; i <= qt->n - 1; ++i){
      qdb_close(qt->nbspq[i], &dberror_close);	/* ignore any error here */
    }    

    free(qt->nbspq);
    free(qt);
    qt = NULL;
  }

  return(qt);
}

int nbspqt_close(nbspqtable_t *qt, int *dberror){

  int status = 0;
  int status1 = 0;
  int dberror1 = 0;
  int i;

  if(qt == NULL)
    return(0);

  if(qt->nbspq != NULL){
    for(i = 0; i <= qt->n - 1; ++i){
      status1 = qdb_close(qt->nbspq[i], &dberror1);
      if(status == 0){
	status = status1;
	*dberror = dberror1;
      }
    }    
    free(qt->nbspq);
  }

  free(qt);

  return(status);
}

int nbspqt_snd(nbspqtable_t *qtable, int type, 
	       void *data, uint32_t data_size, int *dberror){
  /*
   * Returns
   *  0 if there is no error
   * -1 if there is a db error (qtable->dberror has the db error code)
   *  1 the data was queued but the soft limit was reached.
   *  2 the data was not queued because the hard limit has been reached.
   */
  int status = 0;
  struct nbspq_st *nbspq;
  int n;

  assert((type >= 0) && (type <= qtable->n - 1));

  nbspq = qtable->nbspq[type];

  if((status = pthread_mutex_lock(&nbspq->mutex)) != 0){
    *dberror = status;
    return(-1);
  }

  n = nbspq->n;

  if(queue_hard_limit(n, qtable->hardlimit) != 0)
    status = 2;
    
  if(status == 0){
    set_qdb_status_flag(nbspq, QDB_FSTATUS_MAX_HARD, 0);
    status = qdb_snd(nbspq, data, data_size, dberror);
  }

  if(status == 0){
    set_qdb_status_flag(nbspq, QDB_FSTATUS_DBERROR, 0);
    if(queue_soft_limit(n + 1, qtable->softlimit) != 0)
      status = 1;
  }

  if(status == 0)
    set_qdb_status_flag(nbspq, QDB_FSTATUS_MAX_SOFT, 0);
  else if(status == -1)
    set_qdb_status_flag(nbspq, QDB_FSTATUS_DBERROR, 1);
  else if(status == 1)
      set_qdb_status_flag(nbspq, QDB_FSTATUS_MAX_SOFT, 1);
  else if(status == 2)
    set_qdb_status_flag(nbspq, QDB_FSTATUS_MAX_HARD, 1);

  pthread_cond_signal(&nbspq->cond);
  pthread_mutex_unlock(&nbspq->mutex);

  return(status);
}

int nbspqt_rcv(nbspqtable_t *qtable, int type,
	       void **data, uint32_t *data_size, int timeout_ms, int *dberror){
  /*
   * Returns:
   *
   *  0 if there are no errors reading.
   * -1 if there were errors
   *  1 if the queue was empty (after the timeout)
   */
  struct nbspq_st *nbspq;
  int status = 0;
  struct timespec timeout;

  assert((type >= 0) && (type <= qtable->n - 1));

  nbspq = qtable->nbspq[type];

  status = pthread_mutex_lock(&nbspq->mutex);
  if(status != 0){
    *dberror = status;
    return(-1);
  }

  if(timeout_ms >= 0)
    queue_set_timeout((unsigned int)timeout_ms, &timeout);

  while((nbspq->n == 0) && (status == 0)){
    if(timeout_ms >= 0)
      status = pthread_cond_timedwait(&nbspq->cond, &nbspq->mutex, &timeout);
    else
      status = pthread_cond_wait(&nbspq->cond, &nbspq->mutex);
  }

  if(nbspq->n == 0){
    if(status == ETIMEDOUT)
      status = 1;
    else{
      status = -1;
      *dberror = status;
    }
  }else
    status = qdb_rcv(nbspq, data, data_size, dberror);

  pthread_mutex_unlock(&nbspq->mutex);

  return(status);
}

int nbspqt_rcv_cleanup(nbspqtable_t *qtable, int type, int *dberror){
  /*
   * Threads that call nbspqt_rcv() and are cancellable, will wake up
   * with the nbspqt_rcv() mutex locked (Butenhof p.155). Such threads
   * must then call this function from their cleanup handlers.
   */
  struct nbspq_st *nbspq;
  int status = 0;

  assert((type >= 0) && (type <= qtable->n - 1));

  nbspq = qtable->nbspq[type];

  status = pthread_mutex_unlock(&nbspq->mutex);
  if((status != 0) && (status != EPERM)){
    *dberror = status;
    return(-1);
  }

  return(0);
}

static int queue_soft_limit(int n, int softlimit){
  /*
   * Returns:
   *
   * 0 if the size has not been exceeded or if checking has been disabled.
   * 1 if soft limit is reached (n >= soft limit).
   */
  int status = 0;

  if((softlimit > 0) && (n >= softlimit))
      status = 1;

  return(status);
}

static int queue_hard_limit(int n, int hardlimit){
  /*
   * Returns:
   *
   * 0 if the size has not been exceeded or if checking has been disabled.
   * 1 if hard limit reached (n >= hard limit).
   */
  int status = 0;

  if((hardlimit > 0) && (n >= hardlimit))
      status = 1;

  return(status);
}

uint32_t nbspqt_n(nbspqtable_t *qtable, int type){
 
  return((qtable->nbspq[type])->n);
}

int nbspqt_test_qdb_status_flag(nbspqtable_t *qtable, int type, int flag){

  return(test_qdb_status_flag(qtable->nbspq[type], flag));
}

/*
 * One channel functions.
 */
int qdb_open(struct nbspq_st **nbspq, struct qdb_param_st *qdbparam,
	     int *dberror){

  DB *dbp = NULL;
  DB_MPOOLFILE *mpf = NULL;
  uint32_t dbflags = QDB_FLAGS;
  uint32_t mb = (1024 * 1024) * qdbparam->cache_mb;
  struct nbspq_st *q = NULL;
  int f_memory_based = 0;
  int status;

  status = db_create(&dbp, qdbparam->dbenv, 0);
  if(status == 0)
    status = dbp->set_re_len(dbp, qdbparam->reclen);

  if(status == 0){
    if((qdbparam->dbname != NULL) && (qdbparam->dbname[0] != '\0')){
      /*
       * We can set the pagesize also, but the berkely db chooses one according
       * to the operating system parameters if we dont'.
       */
      status = dbp->set_q_extentsize(dbp, qdbparam->extent_size);
    }else{
      /*
       * An in-memory db. Temporary overflow pages kept in memory and,
       * if dbenv == NULL, set the cache size explicitly. There is meaning
       * of page size and extent size in this case, so we deal with the
       * issue of reclaiming the unused memory from the cache explicitly
       * in qdb_rcv() below.
       */
      f_memory_based = 1;
      mpf = dbp->get_mpf(dbp);
      status = mpf->set_flags(mpf, DB_MPOOL_NOFILE, 1);
      if((qdbparam->dbenv == NULL) && (status == 0))	
	status = dbp->set_cachesize(dbp, 0, mb, 1);
    }
  }

  if(status == 0)
    status = dbp->open(dbp, NULL, qdbparam->dbname, NULL, DB_QUEUE, dbflags,
		       qdbparam->mode);

  if(status == 0){
    q = malloc(sizeof(struct nbspq_st));
    if(q == NULL)
      status = errno;
  }

  if(status == 0){
    q->dbp = dbp;
    q->n = 0;
    q->nmax = 0xffffffff;
    q->f_memory_based = f_memory_based;
    q->f_qdb_status = QDB_FSTATUS_OK;
    status = qdb_stat(q);
  }

  if(status == 0){
    status = pthread_mutex_init(&q->mutex, NULL);
    if(status == 0){
      status = pthread_cond_init(&q->cond, NULL);
      if(status != 0)
	pthread_mutex_destroy(&q->mutex);
    }
  }

  if(status != 0){
    if(dbp != NULL)
      dbp->close(dbp, 0);

    if(q != NULL)
      free(q);

    *dberror = status;
    status = -1;
  }

  if(status == 0)
    *nbspq = q;

  return(status);
}

int qdb_close(struct nbspq_st *nbspq, int *dberror){

  int status = 0;
  DB *dbp = NULL;

  assert(nbspq != NULL);

  dbp = nbspq->dbp;    
  if(dbp != NULL)
    status = dbp->close(dbp, 0);

  if(status != 0){
    *dberror = status;
    status = -1;
  }

  pthread_mutex_destroy(&nbspq->mutex);
  pthread_cond_destroy(&nbspq->cond);

  free(nbspq);

  return(status);
}

int qdb_snd(struct nbspq_st *nbspq, void *p, uint32_t size, int *dberror){

  int status;
  DBT key, data;
  uint32_t recno;
  DB* dbp = nbspq->dbp;

  memset(&key, 0 , sizeof(DBT));
  memset(&data, 0 , sizeof(DBT));

  key.data = &recno;
  key.ulen = sizeof(recno);
  key.flags = DB_DBT_USERMEM;
  data.data = p;
  data.size = size;

  if((status = dbp->put(dbp, NULL, &key, &data, DB_APPEND)) == 0)
    ++nbspq->n;
  else{
    *dberror = status;
    status = -1;
  }

  return(status);
}

int qdb_rcv(struct nbspq_st *nbspq, void **p, uint32_t *size, int *dberror){
  /*
   * If (*p == NULL), it allocates space for the data. But if *p is not NULL
   * it reuses the space, updating the size to return the true size of the
   * data.
   */
  int status = 0;
  DBT key, data;
  uint32_t recno;
  uint32_t reclen;
  void *p_copy;
  DB* dbp = nbspq->dbp;

  memset(&key, 0 , sizeof(DBT));
  memset(&data, 0 , sizeof(DBT));

  if((status = dbp->get_re_len(dbp, &reclen)) != 0){
    *dberror = status;
    return(-1);
  }

  if(*p == NULL)
    p_copy = malloc(reclen);
  else{
    if(*size < reclen){
      *dberror = EINVAL;
      return(-1);
    }else
      p_copy = *p;
  }

  if(p_copy == NULL){
    *dberror = errno;
    return(-1);
  }

  key.data = &recno;
  key.ulen = sizeof(recno);
  key.flags = DB_DBT_USERMEM;
  data.data = p_copy;
  data.ulen = reclen;
  data.flags = DB_DBT_USERMEM;

  status = dbp->get(dbp, NULL, &key, &data, DB_CONSUME);
  if(status != 0){
    *dberror = status;
    status = -1;
    if(*p == NULL)
      free(p_copy);
  }else{
    if(*p == NULL)
      *p = p_copy;

    *size = reclen;
    --nbspq->n;

    /*
     * The call to truncate() below is added to handle the case of in-memory
     * queues, in which case the page/extent size mechanism for reclaiming
     * unused space does not operate (both calls return errors when the
     * data base is configured as in-memory db). If this is not done here,
     * or somewhere, then the cache is eventually exhasusted and the
     * the next calls to qdb_send return an error (memory cannot be allocated.)
     */
    if(nbspq->f_memory_based && (nbspq->n == 0))
      status = dbp->truncate(dbp, NULL, NULL, 0);
  }

  return(status);
}

static int qdb_stat(struct nbspq_st *nbspq){

  int status;
  DB_QUEUE_STAT *qs = NULL;
  DB* dbp = nbspq->dbp;

  if((status = dbp->stat(dbp, NULL, &qs, 0)) == 0){
    nbspq->n = qs->qs_ndata;
  }

  if(qs != NULL)
    free(qs);
  
  return(status);
}

uint32_t qdb_n(struct nbspq_st *nqdb){

  return(nqdb->n);
}

/*
 * Utility functions for setting and testing the qdb_status flags.
 */
void set_qdb_status_flag(struct nbspq_st *nqdb, int flag, int onoff){

  if(onoff != 0)
    nqdb->f_qdb_status |= flag;
  else
    nqdb->f_qdb_status &= ~flag;
}

void clear_qdb_status_flag(struct nbspq_st *nqdb){

  nqdb->f_qdb_status = QDB_FSTATUS_OK;
}

int test_qdb_status_flag(struct nbspq_st *nqdb, int flag){

  if(nqdb->f_qdb_status & flag)
    return(1);

  return(0);
}

/*
 * Utility function to set the default pthreads timeout
 */
static void queue_set_timeout(unsigned int timeout_ms,
			      struct timespec *timeout){

  struct timeval tv;

  gettimeofday(&tv, NULL);
  timeout->tv_sec = tv.tv_sec;
  timeout->tv_nsec = tv.tv_usec * 1000;

  timeout->tv_sec += (timeout_ms/1000);
  timeout->tv_nsec += (timeout_ms % 1000)*1000000;
}

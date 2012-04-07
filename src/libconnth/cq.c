/*
 * Copyright (c) 2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include "cq.h"

connqueue_t *connqueue_open(struct connqueue_param_st *cqparam,
			    int *dberror){

  struct connqueue_st *cq;
  struct qdb_param_st qdbparam;
  int soft, hard;

  cq = (connqueue_t*)malloc(sizeof(struct connqueue_st));
  if(cq == NULL)
    return(NULL);

  cq->data_size = 0;
  cq->data_size_max = cqparam->reclen;
  cq->data = malloc(cq->data_size_max);
  if(cq->data == NULL){
    free(cq);
    return(NULL);
  }

  qdbparam.dbenv = NULL;
  qdbparam.dbname = NULL;
  qdbparam.mode = 0;
  qdbparam.extent_size = 0;
  qdbparam.reclen = cqparam->reclen;
  qdbparam.cache_mb = cqparam->cache_mb;
  soft = cqparam->softlimit;
  hard = cqparam->hardlimit;

  *dberror = 0;
  cq->qt = nbspqt_open(1, soft, hard, &qdbparam, dberror);
  if(cq->qt == NULL){
    free(cq->data);
    free(cq);
    return(NULL);
  }

  return(cq);
}

int connqueue_close(connqueue_t *cq, int *dberror){
  /*
   * This function is called by the main thread when deleting
   * a client.
   */
  int status;

  *dberror = 0;

  if(cq->data != NULL)
    free(cq->data);

  status = nbspqt_close(cq->qt, dberror); 
  free(cq);

  return(status);
}

int connqueue_snd(connqueue_t *cq,
		  void *data, uint32_t data_size, int *dberror){
  /*
   * This function returns the same codes as nbspqt_snd (from libqdb),
   * which are:
   *
   *  0 if there is no error
   * -1 if there is a db error (dberror has the db error code)
   *  1 the data was queued but the soft limit was reached.
   *  2 the data was not queued because the hard limit has been reached.
   */

  *dberror = 0;
  return(nbspqt_snd(cq->qt, 0, data, data_size, dberror));
}

int connqueue_rcv(connqueue_t *cq, void **data, uint32_t *data_size,
		  int timeout_ms, int *dberror){
  /*
   * See the note in cq.h about the **data argument returned by this
   * function.
   */
  int status;
  
  *dberror = 0;
  cq->data_size = cq->data_size_max;
  status = nbspqt_rcv(cq->qt,
		    0, &cq->data, &cq->data_size, timeout_ms, dberror);
  if(status == 0){
    *data_size = cq->data_size;
    *data = cq->data;
    if(cq->data_size > cq->data_size_max)
      cq->data_size_max = cq->data_size;
  }

  return(status);
}

int connqueue_rcv_cleanup(connqueue_t *cq, int *dberror){
  /*
   * See the description of nbspqt_rcv_cleanup() in libqdb.
   */
  int status = 0;

  status = nbspqt_rcv_cleanup(cq->qt, 0, dberror);

  return(status);
}

uint32_t connqueue_n(connqueue_t *cq){

  return(nbspqt_n(cq->qt, 0));
}

/*
 * Utility functions to test the qdb_status flags (see libqdb/qdb.h)
 */
int connqueue_test_maxhard_flag(connqueue_t *cq){

  return(nbspqt_test_qdb_status_flag(cq->qt, 0, QDB_FSTATUS_MAX_HARD));
}

int connqueue_test_maxsoft_flag(connqueue_t *cq){

  return(nbspqt_test_qdb_status_flag(cq->qt, 0, QDB_FSTATUS_MAX_SOFT));
}

int connqueue_test_dberror_flag(connqueue_t *cq){

  return(nbspqt_test_qdb_status_flag(cq->qt, 0, QDB_FSTATUS_DBERROR));
}

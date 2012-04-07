/*
 * Copyright (c) 2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef LIBCONNTH_CQ_H
#define LIBCONNTH_CQ_H

#include "../libqdb/qdb.h"

/*
 * Use a permanent buffer where to receive the data from the queue.
 * In connqueue_rcv(), the **data returned points to the internal storage
 * and should not be free()'d explicitly ever. It is destroyed when
 * the conqueue_t is closed (by the main thread).
 */
typedef struct connqueue_st {
  struct nbsp_qtable_st *qt;
  void *data;
  uint32_t data_size_max;	/* allocated size */
  uint32_t data_size;		/* size of returned data */
} connqueue_t;

/* Parameters for creating the connection queues. */
struct connqueue_param_st {
  uint32_t reclen;
  uint32_t cache_mb;
  int softlimit;
  int hardlimit;
};

/*
 * The first three (open, close, snd) functions are called by the main thread,
 * while rcv and rcv_cleanup is called by the individual client threads.
 * connqueue_n(connqueue_t *cq) returns the current number of
 * elements in the queue. It is not accurate because the queue
 * is not locked while the number is read. It is meant to be used just
 * for reporting.
 */
connqueue_t *connqueue_open(struct connqueue_param_st *cqparam,
			    int *dberror);
int connqueue_close(connqueue_t *cq, int *dberror);
int connqueue_snd(connqueue_t *cq,
		  void *data, uint32_t data_size, int *dberror);
int connqueue_rcv(connqueue_t *cq, void **data, uint32_t *data_size,
		  int timeout_ms, int *dberror);
int connqueue_rcv_cleanup(connqueue_t *cq, int *dberror);
uint32_t connqueue_n(connqueue_t *cq);
int connqueue_test_maxhard_flag(connqueue_t *cq);
int connqueue_test_maxsoft_flag(connqueue_t *cq);
int connqueue_test_dberror_flag(connqueue_t *cq);

#endif

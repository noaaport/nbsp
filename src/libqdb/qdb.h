/*
 * Copyright (c) 2005-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef LIBQDB_H
#define LIBQDB_H

#include <pthread.h>
#include <db.h>

/* queue status flags */
#define QDB_FSTATUS_OK		0
#define QDB_FSTATUS_MAX_SOFT	0x0001
#define QDB_FSTATUS_MAX_HARD	0x0002
#define QDB_FSTATUS_DBERROR	0x0004

/*
 * One channel
 */
struct nbspq_st {
  DB *dbp;
  uint32_t n;
  uint32_t nmax;	/* from db: 0xffffffff */
  pthread_cond_t  cond;
  pthread_mutex_t mutex;
  int f_memory_based;
  int f_qdb_status;	/* queue status flags */
};

/*
 * Table of channels
 */
struct nbsp_qtable_st {
  int n;                        /* number of queues */
  int softlimit;
  int hardlimit;
  uint32_t reclen;		/* record length */
  void *appdata;                /* for application use */
  struct nbspq_st **nbspq;	/* array of pointers to channels */
};
typedef struct nbsp_qtable_st nbspqtable_t;

/*
 * Configuration parameters of the queue db. For in-memory dbs, dbname is null
 * and mode and extent_size are ignored. In addition if the dbenv is null
 * also, then the cache size of each queue is set explicitly, otherwise
 * we assume that it has been set in the dbenv.
 */
struct qdb_param_st {
  DB_ENV *dbenv;
  char *dbname;
  int mode;
  uint32_t extent_size;
  uint32_t reclen;
  uint32_t cache_mb;	/* used only if dbenv == NULL, ignored otherwise */
};

/*
 * Functions that operate on a table of channels. 
 * Except for nbspqt_open, in case of error the functions return -1
 * or a positive integer.
 */
nbspqtable_t *nbspqt_open(u_int8_t num_queues, int softlimit, int hardlimit,
			  struct qdb_param_st *qdbparam, int *dberror);
int nbspqt_close(nbspqtable_t *qt, int *dberror);
int nbspqt_snd(nbspqtable_t *qtable, int type, 
	       void *data, uint32_t data_size, int *dberror);
int nbspqt_rcv(nbspqtable_t *qtable, int type,
	       void **data, uint32_t *data_size, int timeout_ms, int *dberror);
int nbspqt_rcv_cleanup(nbspqtable_t *qtable, int type, int *dberror);

/*
 * Functions that operate on a channel.
 */
int qdb_open(struct nbspq_st **nbspq, struct qdb_param_st *qdbparam,
	     int *dberror);
int qdb_close(struct nbspq_st *nqdb, int *dberror);
int qdb_snd(struct nbspq_st *nqdb, void *p, uint32_t size, int *dberror);
int qdb_rcv(struct nbspq_st *nqdb, void **p, uint32_t *size, int *dberror);

/*
 * These functions return the current number of
 * elements in the queue. It is not accurate because the queue
 * is not locked while the number is read. It is meant to be used just
 * for reporting.
 */
uint32_t qdb_n(struct nbspq_st *nqdb);
uint32_t nbspqt_n(nbspqtable_t *qtable, int type);

/*
 * Utility functions for setting and testing the qdb_status flags.
 */
void set_qdb_status_flag(struct nbspq_st *nqdb, int flag, int onoff);
void clear_qdb_status_flag(struct nbspq_st *nqdb);
int  test_qdb_status_flag(struct nbspq_st *nqdb, int flag);
int nbspqt_test_qdb_status_flag(nbspqtable_t *qtable, int type, int flag);

#endif

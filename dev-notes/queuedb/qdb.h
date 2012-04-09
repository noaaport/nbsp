/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef QDB_H
#define QDB_H

#include <pthread.h>
#include <db.h>

#define QDB_FLAGS	(DB_CREATE | DB_THREAD)

struct nbspqdb_st{
  DB *dbp;
  int n;
  int nmax;
  pthread_mutex_t mutex;
};

struct nbs_qtable_st {
  int n;                        /* number of queues */
  int softlimit;
  int hardlimit;
  void *appdata;                /* for application use */
  struct nbspqdb_st *q;		/* array of queues (channels) */
};

int qdb_open(struct nbspqdb_st **nqdb, char *dbfname, u_int32_t recsize);
int qdb_close(struct nbspqdb_st *nqdb);
int qdb_send(struct nbspqdb_st *nqdb, void *p, u_int32_t size);
int qdb_rcv(struct nbspqdb_st *nqdb, void *p, u_int32_t size);

#endif

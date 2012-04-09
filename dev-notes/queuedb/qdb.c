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
#include <errno.h>
#include "qdb.h"

int qdb_open(struct nbspqdb_st **nqdb, char *dbfname, u_int32_t reclen){

  DB *dbp = NULL;
  int status;
  u_int32_t dbflags = QDB_FLAGS;

  *nqdb = NULL;

  status = db_create(&dbp, NULL, 0);
  if(status == 0)
    status = dbp->set_re_len(dbp, reclen);

  if(status == 0)
    status = dbp->open(dbp, NULL, dbfname, NULL, DB_QUEUE, dbflags, 0);

  if(status == 0){
    *nqdb = malloc(sizeof(struct nbspqdb_st));
    if(*nqdb == NULL)
      status = errno;
  }

  if(status == 0)
    status = pthread_mutex_init(&((*nqdb)->mutex), NULL);

  if(status == 0){
    (*nqdb)->dbp = dbp;
    (*nqdb)->n = 0;
    (*nqdb)->nmax = 0;
  }

  if(status != 0){
    if(dbp != NULL)
      dbp->close(dbp, 0);

    if(*nqdb != NULL)
      free(*nqdb);
  }
 
  return(status);
}

int qdb_close(struct nbspqdb_st *nqdb){

  int status = 0;
  DB *dbp = NULL;

  assert(nqdb != NULL);

  dbp = nqdb->dbp;    
  if(dbp != NULL)
    status = dbp->close(dbp, 0);

  pthread_mutex_destroy(&nqdb->mutex);
  
  return(status);
}

int qdb_send(struct nbspqdb_st *nqdb, void *p, u_int32_t size){

  int status;
  DBT key, data;
  u_int32_t recno;
  DB* dbp = nqdb->dbp;

  memset(&key, 0 , sizeof(DBT));
  memset(&data, 0 , sizeof(DBT));

  key.data = &recno;
  key.ulen = sizeof(recno);
  key.flags = DB_DBT_USERMEM;
  data.data = p;
  data.size = size;

  status = dbp->put(dbp, NULL, &key, &data, DB_APPEND);

  if(status == 0){
    pthread_mutex_lock(&nqdb->mutex);
    ++nqdb->n;
    pthread_mutex_unlock(&nqdb->mutex);
  }

  return(status);
}

int qdb_rcv(struct nbspqdb_st *nqdb, void *p, u_int32_t size){

  int status;
  DBT key, data;
  u_int32_t recno;
  DB* dbp = nqdb->dbp;

  memset(&key, 0 , sizeof(DBT));
  memset(&data, 0 , sizeof(DBT));

  key.data = &recno;
  key.ulen = sizeof(recno);
  key.flags = DB_DBT_USERMEM;
  data.data = p;
  data.ulen = size;
  data.flags = DB_DBT_USERMEM;

  status = dbp->get(dbp, NULL, &key, &data, DB_CONSUME);
  if(status == 0){
    pthread_mutex_lock(&nqdb->mutex);
    --nqdb->n;
    pthread_mutex_unlock(&nqdb->mutex);
  }

  return(status);
}

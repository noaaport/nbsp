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
#include "nbspdb.h"

static int nbspdb_write_record(DB *dbp, struct nbspdb_prec_st *prec);
static int nbspdb_read_record_byname(DB *dbp, struct nbspdb_prec_st *prec);
static int nbspdb_read_record_byseq(DB *sdbp, struct nbspdb_prec_st *prec);
static int nbspdb_write_record2(DB *dbp,
			       char *fname, struct nbspdb_pdata_st *pdata);
static int nbspdb_update_record(DBC *cursor,
				char *fname, struct nbspdb_pdata_st *pdata);

static int get_key_callback(DB *sdbp, const DBT *pkey,
			    const DBT *pdata, DBT *skey);

int nbspdb_open(struct nbspdb_st **nbspdb, char *dbfname, char *sdbfname,
		u_int32_t dbflags){

  DB *dbp = NULL;
  DB *sdbp = NULL;
  DBC *cursor = NULL;
  int status;

  status = db_create(&dbp, NULL, 0);
  if(status == 0)
    status = db_create(&sdbp, NULL, 0);

  if(status == 0)
    status = dbp->open(dbp, NULL, dbfname, NULL, DB_BTREE, dbflags, 0);

  if(status == 0)
    status = sdbp->open(sdbp, NULL, sdbfname, NULL, DB_BTREE, dbflags, 0);

  if(status == 0)
    status = dbp->associate(dbp, NULL, sdbp, get_key_callback, 0);

  if(status == 0)
    status = dbp->cursor(dbp, NULL, &cursor, 0);

  if(status == 0){
    *nbspdb = malloc(sizeof(struct nbspdb_st));
    if(*nbspdb == NULL)
      status = errno;
  }

  if(status == 0){
    (*nbspdb)->dbp = dbp;
    (*nbspdb)->sdbp = sdbp;
    (*nbspdb)->cursor = cursor;
  }

  if(status != 0){
    if(cursor != NULL)
      cursor->c_close(cursor);

    if(sdbp != NULL)
      sdbp->close(sdbp, 0);

    if(dbp != NULL)
      dbp->close(dbp, 0);
  }
 
  return(status);
}

int nbspdb_close(struct nbspdb_st *nbspdb){

  int status = 0, status1 = 0;
  DB *dbp = NULL;
  DB *sdbp = NULL;
  DBC *cursor = NULL;

  assert(nbspdb != NULL);

  dbp = nbspdb->dbp;
  sdbp = nbspdb->sdbp;
  cursor = nbspdb->cursor;

  if(cursor != NULL)
    cursor->c_close(cursor);

  if(sdbp != NULL)
    status = sdbp->close(sdbp, 0);
    
  if(dbp != NULL){
    status1 = dbp->close(dbp, 0);
    if(status == 0)
      status = status1;
  }
  
  return(status);
}

int nbspdb_put(struct nbspdb_st *nbspdb, struct nbspdb_prec_st *prec){

  int status;

  status = nbspdb_write_record(nbspdb->dbp, prec);

  return(status);
}

int nbspdb_get_byname(struct nbspdb_st *nbspdb, struct nbspdb_prec_st *prec){

  int status;

  status = nbspdb_read_record_byname(nbspdb->dbp, prec);

  return(status);
}

int nbspdb_get_byseq(struct nbspdb_st *nbspdb, struct nbspdb_prec_st *prec){

  int status;

  status = nbspdb_read_record_byseq(nbspdb->sdbp, prec);

  return(status);
}

int nbspdb_update(struct nbspdb_st *nbspdb, struct nbspdb_prec_st *prec){

  int status;

  status = nbspdb_update_record(nbspdb->cursor, prec->fname, &prec->data);

  return(status);
}

int nbspdb_put2(struct nbspdb_st *nbspdb,
		char *fname, unsigned int seqnum, int fstatus){

  int status;
  struct nbspdb_pdata_st pdata;

  pdata.seq = seqnum;
  pdata.status = fstatus;

  status = nbspdb_write_record2(nbspdb->dbp, fname, &pdata);

  return(status);
}

int nbspdb_update2(struct nbspdb_st *nbspdb,
		   char *fname, unsigned int seqnum, int fstatus){

  int status;
  struct nbspdb_pdata_st pdata;

  pdata.seq = seqnum;
  pdata.status = fstatus;

  status = nbspdb_update_record(nbspdb->cursor, fname, &pdata);

  return(status);
}

static int nbspdb_write_record(DB *dbp, struct nbspdb_prec_st *prec){

  int status;

  status = nbspdb_write_record2(dbp, prec->fname, &prec->data);

  return(status);
}

static int nbspdb_write_record2(DB *dbp,
			       char *fname, struct nbspdb_pdata_st *pdata){
  int status;
  DBT key, data;

  memset(&key, 0 , sizeof(DBT));
  memset(&data, 0 , sizeof(DBT));

  key.data = fname;
  key.size = strlen(fname) + 1;
  data.data = pdata;
  data.size = sizeof(struct nbspdb_pdata_st);

  status = dbp->put(dbp, NULL, &key, &data, 0);

  return(status);
}

static int nbspdb_read_record_byname(DB *dbp, struct nbspdb_prec_st *prec){

  int status;
  DBT key, data;

  memset(&key, 0 , sizeof(DBT));
  memset(&data, 0 , sizeof(DBT));

  key.data = prec->fname;
  key.size = strlen(prec->fname) + 1;
  data.data = &prec->data;
  data.ulen = sizeof(struct nbspdb_pdata_st);
  data.flags = DB_DBT_USERMEM;

  status = dbp->get(dbp, NULL, &key, &data, 0);

  return(status);
}

static int nbspdb_read_record_byseq(DB *sdbp, struct nbspdb_prec_st *prec){

  int status;
  DBT search_key;
  DBT key, data;	/* primary key and data */
  unsigned int seq;

  memset(&search_key, 0 , sizeof(DBT));
  memset(&key, 0 , sizeof(DBT));
  memset(&data, 0 , sizeof(DBT));

  seq = prec->data.seq;
  search_key.data = &seq;
  search_key.size = sizeof(unsigned int); 

  key.data = prec->fname;
  key.ulen = FNAME_SIZE + 1;
  key.flags = DB_DBT_USERMEM;

  data.data = &prec->data;
  data.ulen = sizeof(struct nbspdb_pdata_st);
  data.flags = DB_DBT_USERMEM;

  status = sdbp->pget(sdbp, NULL, &search_key, &key, &data, 0);

  return(status);
}

static int nbspdb_update_record(DBC *cursor,
				char *fname, struct nbspdb_pdata_st *pdata){

  int status;
  DBT key, data;

  memset(&key, 0 , sizeof(DBT));
  memset(&data, 0 , sizeof(DBT));

  key.data = fname;
  key.size = strlen(fname) + 1;

  status = cursor->c_get(cursor, &key, &data, DB_SET);

  if(status == 0){
    data.data = pdata;
    data.size = sizeof(struct nbspdb_pdata_st);
    status = cursor->c_put(cursor, &key, &data, DB_CURRENT);
  }

  return(status);
}

static int get_key_callback(DB *sdbp, const DBT *key, const DBT *data,
			  DBT *skey){

  struct nbspdb_pdata_st *pdata;

  pdata = data->data;
  memset(skey, 0, sizeof(DBT));
  skey->data = &pdata->seq;
  skey->size = sizeof(unsigned int);

  return(0);
}


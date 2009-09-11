/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "util.h"
#include "nbspdb.h"

#define NBSPDB_FLAGS	(DB_CREATE | DB_THREAD)

static int nbspdb_write_record(DB *dbp, struct nbspdb_data_st *data);
static int nbspdb_read_record(DB *sdbp, struct nbspdb_data_st *data);
static int nbspdb_put(struct nbspdb_st *nbspdb,
		      unsigned int seqnum, int status);
/*
 * The function to read from the secondary db, and the callback.
 */
static int get_seqnum(DB *sdbp, const DBT *pkey, const DBT *pdata, DBT *skey);

int nbspdb_open(struct nbspdb_st **nbspdb, DB_ENV *dbenv,
		unsigned int numslots, char *dbfname, int mode){

  int status;
  char *fullname = NULL;
  int fullname_size = 0;
  DB *dbp = NULL;
  DB *sdbp = NULL;
  uint32_t dbflags = NBSPDB_FLAGS;

  assert(numslots > 0);

  /*
   * If dbfname == NULL, the database is purely memory based without
   * a backing file.
   */
  if(valid_str(dbfname) == 1){
    fullname_size = strlen(dbfname) + 2;		/* '\0' + digit */
    fullname = malloc(fullname_size);	
    if(fullname == NULL)
      return(errno);

    strncpy(fullname, dbfname, fullname_size);
    fullname[fullname_size - 2] = '0';
  }

  status = db_create(&dbp, dbenv, 0);
  if(status == 0)
    status = dbp->open(dbp, NULL, fullname, NULL, DB_RECNO, dbflags, mode);

  if(status == 0){
    if(fullname != NULL)
      fullname[fullname_size - 2] = '1';

    status = db_create(&sdbp, dbenv, 0);
    if(status == 0)
      status = sdbp->open(sdbp, NULL, fullname, NULL, DB_BTREE, dbflags, mode);
  }

  if(status == 0)
    status = dbp->associate(dbp, NULL, sdbp, get_seqnum, 0);

  if(status == 0){
    *nbspdb = malloc(sizeof(struct nbspdb_st));
    if(*nbspdb == NULL)
      status = errno;
  }

  if(status == 0){
    (*nbspdb)->dbp = dbp;
    (*nbspdb)->sdbp = sdbp;
    (*nbspdb)->numslots = 0;
    (*nbspdb)->numslots_max = numslots;
  }

  if(status != 0){
    if(sdbp != NULL)
      sdbp->close(sdbp, 0);

    if(dbp != NULL)
      dbp->close(dbp, 0);
  }

  if(fullname != NULL)
    free(fullname);
 
  return(status);
}

int nbspdb_close(struct nbspdb_st *nbspdb){

  int status = 0;
  DB *dbp = NULL;

  assert(nbspdb != NULL);

  dbp = nbspdb->sdbp;
  if(dbp != NULL)
    status = dbp->close(dbp, 0);

  dbp = nbspdb->dbp;
  if(dbp != NULL)
    status = dbp->close(dbp, 0);
  
  return(status);
}

int nbspdb_put_ok(struct nbspdb_st *nbspdb, unsigned int seqnum){

  int status;

  status = nbspdb_put(nbspdb, seqnum, 0);

  return(status);
}

int nbspdb_put_fail(struct nbspdb_st *nbspdb, unsigned int seqnum){

  int status;

  status = nbspdb_put(nbspdb, seqnum, 1);

  return(status);
}

int nbspdb_get(struct nbspdb_st *nbspdb, struct nbspdb_data_st *data){
  /*
   * Searches the seqnumber in the secondary db.
   */
  int status;

  status = nbspdb_read_record(nbspdb->sdbp, data);

  return(status);
}

static int nbspdb_put(struct nbspdb_st *nbspdb,
		      unsigned int seqnum, int data_status){

  int status;
  struct nbspdb_data_st data;
  uint32_t count;

  data.seqnum = seqnum;
  data.status = data_status;

  status = nbspdb_write_record(nbspdb->dbp, &data);
  ++nbspdb->numslots;
  if(nbspdb->numslots == nbspdb->numslots_max){
    status = (nbspdb->dbp)->truncate(nbspdb->dbp, NULL, &count, 0);
    if(status == 0)
      nbspdb->numslots = 0;
  }

  return(status);
}

static int nbspdb_write_record(DB *dbp, struct nbspdb_data_st *data){

  int status;
  DBT key, pdata;
  uint32_t recno;

  memset(&key, 0 , sizeof(DBT));
  memset(&pdata, 0 , sizeof(DBT));

  key.data = &recno;
  key.ulen = sizeof(recno);
  key.flags = DB_DBT_USERMEM;
  pdata.data = data;
  pdata.size = sizeof(struct nbspdb_data_st);

  status = dbp->put(dbp, NULL, &key, &pdata, DB_APPEND);

  return(status);
}

static int nbspdb_read_record(DB *sdbp, struct nbspdb_data_st *data){
  /*
   * Searches the secondary db by the seqnumber in the prec.
   */
  int status;
  DBT key;
  DBT pdata;

  memset(&key, 0 , sizeof(DBT));
  memset(&pdata, 0 , sizeof(DBT));

  key.data = &data->seqnum;
  key.size = sizeof(data->seqnum);
  pdata.data = data;
  pdata.ulen = sizeof(struct nbspdb_data_st);
  pdata.flags = DB_DBT_USERMEM;

  status = sdbp->get(sdbp, NULL, &key, &pdata, 0);

  return(status);
}

static int get_seqnum(DB *sdbp, const DBT *pkey, const DBT *pdata, DBT *skey){
  /*
   * Callback for secondary db.
   */
  struct nbspdb_data_st *nbspdata;
  
  /* Get the primary data */
  nbspdata = (struct nbspdb_data_st*)pdata->data;

  /* Set the secondary key */
  memset(skey, 0, sizeof(DBT));
  skey->data = &nbspdata->seqnum;
  skey->size = sizeof(nbspdata->seqnum);

  return(0);
}

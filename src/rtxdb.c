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
#include "rtxdb.h"

#define NBSP_RTXDB_FLAGS	(DB_CREATE | DB_THREAD)

static int rtxdb_write_record(DB *dbp, struct nbsp_rtxdb_rec_st *rec);
static int rtxdb_read_record(DB *dbp, struct nbsp_rtxdb_rec_st *rec);
static int rtxdb_read_record2(DB *dbp, unsigned int seqnum,
			      struct nbsp_rtxdb_data_st *rtxdata);
static int rtxdb_write_record2(DB *dbp, unsigned int seqnum,
			       struct nbsp_rtxdb_data_st *rtxdata);
static int rtxdb_compare(DB *dbp, const DBT *a, const DBT *b);

int nbsp_rtxdb_open(struct nbsp_rtxdb_st **rtxdb, DB_ENV *dbenv,
		    char *dbfname, mode_t mode){

  int status;		
  DB *dbp1 = NULL;
  DB *dbp2 = NULL;
  uint32_t dbflags = NBSP_RTXDB_FLAGS;
  char *fullname = NULL;
  int fullname_size = 0;

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
    fullname[fullname_size - 2] = '1';
  }
     
  status = db_create(&dbp1, dbenv, 0);

  if(status == 0)
    status = dbp1->open(dbp1, NULL, fullname, NULL, DB_BTREE, dbflags, mode);

  if(status == 0)
    dbp1->set_bt_compare(dbp1, rtxdb_compare);

  if(status == 0){
    if(fullname != NULL)
      fullname[fullname_size - 2] = '2';

    status = db_create(&dbp2, dbenv, 0);
    if(status == 0)
      dbp2->set_bt_compare(dbp2, rtxdb_compare);

    if(status == 0)
      status = dbp2->open(dbp2, NULL, fullname, NULL, DB_BTREE, dbflags, mode);
  }

  if(status == 0){
    *rtxdb = malloc(sizeof(struct nbsp_rtxdb_st));
    if(*rtxdb == NULL)
      status = errno;
  }

  if(status == 0){
    (*rtxdb)->dbp1 = dbp1;
    (*rtxdb)->dbp2 = dbp2;
    (*rtxdb)->dbp_c = dbp1;
    (*rtxdb)->dbp_o = dbp2;
  }

  if(status != 0){
    if(dbp1 != NULL)
      dbp1->close(dbp1, 0);

    if(dbp2 != NULL)
      dbp2->close(dbp2, 0);
  }

  if(fullname != NULL)
    free(fullname);
 
  return(status);
}

int nbsp_rtxdb_close(struct nbsp_rtxdb_st *rtxdb){

  int status = 0;
  DB *dbp = NULL;

  assert(rtxdb != NULL);

  dbp = rtxdb->dbp1;
  if(dbp != NULL)
    status = dbp->close(dbp, 0);

  dbp = rtxdb->dbp2;
  if(dbp != NULL)
    status = dbp->close(dbp, 0);
  
  return(status);
}

int nbsp_rtxdb_put(struct nbsp_rtxdb_st *rtxdb,
		   unsigned int seqnum, int fstatus){

  int status;
  struct nbsp_rtxdb_rec_st rec;

  rec.data.fstatus = fstatus;
  rec.seqnum = seqnum;
  status = rtxdb_write_record(rtxdb->dbp_c, &rec);

  return(status);
}

int nbsp_rtxdb_get(struct nbsp_rtxdb_st *rtxdb,
		   unsigned seqnum, int *fstatus){
  /*
   * The function returns
   *
   * 0 => no error, or a db error code as usual (including "not found").
   *
   * When the function returns 0, then fstatus contains:
   *
   * 0 => seqnum appear in the db and fstatus is 0.
   * 1 => seqnum appears in the db and fstatus is 1.
   */
  int status;
  struct nbsp_rtxdb_rec_st rec;
  
  rec.seqnum = seqnum;

  status = rtxdb_read_record(rtxdb->dbp_c, &rec);
  if(status != 0)
    status = rtxdb_read_record(rtxdb->dbp_o, &rec);

  if(status == 0)
    *fstatus = rec.data.fstatus;

  return(status);
}

int nbsp_rtxdb_truncate(struct nbsp_rtxdb_st *rtxdb){

  int status = 0;
  u_int32_t count;
  DB *dbp = NULL;

  /*
   * The old db becomes the current one, which is truncated.
   */
  dbp = rtxdb->dbp_o;
  rtxdb->dbp_o = rtxdb->dbp_c;
  rtxdb->dbp_c = dbp;

  if(dbp != NULL)
    status = dbp->truncate(dbp, NULL, &count, 0);

  return(status);
}

static int rtxdb_write_record(DB *dbp, struct nbsp_rtxdb_rec_st *rec){

  int status;

  status = rtxdb_write_record2(dbp, rec->seqnum, &rec->data);

  return(status);
}

static int rtxdb_write_record2(DB *dbp, unsigned int seqnum,
			       struct nbsp_rtxdb_data_st *rtxdata){
  int status;
  DBT key, data;

  memset(&key, 0 , sizeof(DBT));
  memset(&data, 0 , sizeof(DBT));

  key.data = (void*)&seqnum;
  key.size = sizeof(seqnum);
  data.data = (void*)rtxdata;
  data.size = sizeof(struct nbsp_rtxdb_data_st);

  status = dbp->put(dbp, NULL, &key, &data, 0);

  return(status);
}

static int rtxdb_read_record(DB *dbp, struct nbsp_rtxdb_rec_st *rec){

  int status;

  status = rtxdb_read_record2(dbp, rec->seqnum, &rec->data);

  return(status);
}

static int rtxdb_read_record2(DB *dbp,
			      unsigned int seqnum,
			      struct nbsp_rtxdb_data_st *rtxdata){
  int status;
  DBT key, data;

  memset(&key, 0 , sizeof(DBT));
  memset(&data, 0 , sizeof(DBT));

  key.data = (void*)&seqnum;
  key.size = sizeof(seqnum);
  data.data = rtxdata;
  data.ulen = sizeof(struct nbsp_rtxdb_data_st);
  data.flags = DB_DBT_USERMEM;

  status = dbp->get(dbp, NULL, &key, &data, 0);

  return(status);
}

static int rtxdb_compare(DB *dbp __attribute__((unused)),
			 const DBT *a, const DBT*b){
  unsigned int ai, bi;

  /*
   * Returns:
   * -1 0 if a < b
   *  0 if a = b
   *  1 if a > b
   */
  memcpy(&ai, a->data, sizeof(ai));
  memcpy(&bi, b->data, sizeof(bi));

  if(ai < bi)
    return(-1);
  else if(ai > bi)
    return(1);

  return(0);
}

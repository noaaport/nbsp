/*
 * Copyright (c) 2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include "globals.h"
#include "err.h"
#include "const.h"
#include "dbstats.h"
#include "dbpanic.h"
#include "libspoolbdb/mspoolbdb.h"
#include "spooltype.h"
#include "nbspmspoolbdb.h"

/*
 * The spoolbdb functions return either 0, an errno or a dberror code.
 * The functions in this file write an appropriate message in case of
 * of an error, return -1 in that case, or return 0 if there is no error.
 */

/* The key will be the sequence number of the file name */
#define FKEY_SPOOLBDB_SIZE	FSEQNUM_SIZE

static int get_fpath_key(char *fpath, char *spoolbdb_fkey);

/* spool functions */
static int _nbsp_mspoolbdb_destroy(struct mspoolbdb_st *mspool);
static int _nbsp_mspoolbdb_insert(struct mspoolbdb_st *mspool,
				  char *fpath, void *fdata, size_t fdata_size);
/* slot */
static int _nbsp_mspoolbdb_open(struct mspoolbdb_st *mspool, char *fpath);
static int _nbsp_mspoolbdb_close(struct mspoolbdb_st *mspool, int sd);
static size_t _nbsp_mspoolbdb_read(struct mspoolbdb_st *mspool,
				   int sd, void *data, size_t data_size);
static size_t _nbsp_mspoolbdb_datasize(struct mspoolbdb_st *mspool, int sd);
static int _nbsp_mspoolbdb_fpathsize(struct mspoolbdb_st *mspool,
				     char *fpath, size_t *size);

static int get_fpath_key(char *fpath, char *spoolbdb_fkey){
  /*
   * The function assumes that spoolbdb_fkey is at least of size
   * (FKEY_SPOOLBDB_SIZE + 1).
   */
  char *p;

  p = strrchr(fpath, FNAME_SEQNUM_SEPCHAR);
  if(p == NULL){
    log_errx("get_fpath_key(): no key in fpath %s", fpath);
    return(1);
  }

  ++p;
  if(strlen(p) >= FKEY_SPOOLBDB_SIZE){
    log_errx("get_fpath_key(): key too long in fpath %s:", fpath);
    return(1);
  }

  strncpy(spoolbdb_fkey, p, FKEY_SPOOLBDB_SIZE + 1);
  
  return(0);
}

int nbsp_mspoolbdb_create(void){

  int status;

  status = mspoolbdb_create(&g.mspoolbdb,
			    g.mspoolbdb_dbcache_mb,
			    g.mspoolbdb_maxsize_per128,
			    g.mspoolbdb_ndb,
			    g.mspoolbdb_nslots);

  if(status == 0)
    status = mspoolbdb_set_event_notify(g.mspoolbdb, nbsp_db_event_callback);

  if(status != 0){
    log_errx("Cannot create memory spool bdb. %s", db_strerror(status));
    return(-1);
  }

  return(0);
}

int nbsp_cspoolbdb_create(void){

  int status;
  int f_rdonly = 0;	/* set the DB_CREATE flag to the underlying db */
  int f_mpool_nofile = 0;

  if(spooltype_cspool() == 0){
    log_errx("Invalid value of spooltype in nbsp_cspoolbdb_create()");
    return(-1);
  }

  f_mpool_nofile = 0;
  if(spooltype_cspool_nofile())
    f_mpool_nofile = 1;

  status = cspoolbdb_create(&g.cspoolbdb,
			    g.cspoolbdb_dir, g.cspoolbdb_mode,
			    g.cspoolbdb_dbcache_mb, g.cspoolbdb_maxsize_per128,
			    g.cspoolbdb_name, g.cspoolbdb_mode,
			    g.cspoolbdb_ndb,
			    g.cspoolbdb_nslots,
			    f_mpool_nofile, f_rdonly);
  if(status != 0){
    log_errx("Cannot create spool cache bdb. %s", db_strerror(status));
    return(-1);
  }

  return(0);
}

int nbsp_mspoolbdb_destroy(void){

  int status = 0;

  status = _nbsp_mspoolbdb_destroy(g.mspoolbdb);
  g.mspoolbdb = NULL;

  return(status);
}

int nbsp_cspoolbdb_destroy(void){

  int status = 0;

  status = _nbsp_mspoolbdb_destroy(g.cspoolbdb);
  g.cspoolbdb = NULL;

  return(status);
}

int nbsp_mspoolbdb_insert(char *fpath, void *fdata, size_t fdata_size){

  return(_nbsp_mspoolbdb_insert(g.mspoolbdb, fpath, fdata, fdata_size));
}

int nbsp_cspoolbdb_insert(char *fpath, void *fdata, size_t fdata_size){

  return(_nbsp_mspoolbdb_insert(g.cspoolbdb, fpath, fdata, fdata_size));
}

/* slot */
int nbsp_mspoolbdb_open(char *fpath){
  /*
   * Returns:
   *
   * -1 => error
   * -2 => not found in bdb (DB_NOTFOUND)
   * sd >= 0
   */

  return(_nbsp_mspoolbdb_open(g.mspoolbdb, fpath));
}

int nbsp_cspoolbdb_open(char *fpath){
  /*
   * Returns:
   *
   * -1 => error
   * -2 => not found in bdb
   */

  return(_nbsp_mspoolbdb_open(g.cspoolbdb, fpath));
}

int nbsp_mspoolbdb_close(int sd){

  return(_nbsp_mspoolbdb_close(g.mspoolbdb, sd));
}

int nbsp_cspoolbdb_close(int sd){

  return(_nbsp_mspoolbdb_close(g.cspoolbdb, sd));
}

size_t nbsp_mspoolbdb_read(int sd, void *data, size_t data_size){

  return(_nbsp_mspoolbdb_read(g.mspoolbdb, sd, data, data_size));
}

size_t nbsp_cspoolbdb_read(int sd, void *data, size_t data_size){

  return(_nbsp_mspoolbdb_read(g.cspoolbdb, sd, data, data_size));
}

size_t nbsp_mspoolbdb_datasize(int sd){

  return(_nbsp_mspoolbdb_datasize(g.mspoolbdb, sd));
}

size_t nbsp_cspoolbdb_datasize(int sd){

  return(_nbsp_mspoolbdb_datasize(g.cspoolbdb, sd));
}

int nbsp_mspoolbdb_fpathsize(char *fpath, size_t *size){

  return(_nbsp_mspoolbdb_fpathsize(g.mspoolbdb, fpath, size));
}

int nbsp_cspoolbdb_fpathsize(char *fpath, size_t *size){

  return(_nbsp_mspoolbdb_fpathsize(g.cspoolbdb, fpath, size));
}

/*
 * private functions
 */
static int _nbsp_mspoolbdb_destroy(struct mspoolbdb_st *mspool){

  int status;

  if(mspool == NULL)
    return(0);

  status = mspoolbdb_destroy(mspool);

  if(status != 0){
    log_errx("Error closing mspoolbdb. %s", db_strerror(status));
    return(-1);
  }else
    log_info("Closed mspool bdb.");

  return(status);
}

static int _nbsp_mspoolbdb_insert(struct mspoolbdb_st *mspool,
			   char *fpath, void *fdata, size_t fdata_size){
  int status;
  char fkey[FKEY_SPOOLBDB_SIZE + 1];

  status = get_fpath_key(fpath, fkey);
  assert(status == 0);
  if(status != 0)
    return(-1);

  if(fdata_size > (size_t)UINT32_MAX){
    log_errx("Cannot insert %s in mspoolbdb. Data size too large.", fpath);
    return(-1);
  }

  status = mspoolbdb_insert(mspool, fkey, fdata, (uint32_t)fdata_size);

  if(status == ENOMEM){
    log_errx("Trying to make room for %s[%u bytes] in mspoolbdb.",
	     fkey, fdata_size);
    /*
     * Try a two-step recovery. First truncate the oldest db, and if that
     * is not enough try truncating all of them. This procesure assumes
     * that the reason for the error is a bdb memory error (status == ENOMEM)
     * and that releasing some memory solves the problem.
     */
    status = mspoolbdb_truncate_oldest(mspool);
    if(status == 0)
      status = mspoolbdb_insert(mspool, fkey, fdata, (uint32_t)fdata_size);

    if(status == 0)
      log_info("Truncated oldest db in mspoolbdb. Inserted %s.", fkey);
  }

  if(status == ENOMEM){
    status = mspoolbdb_truncate(mspool);
    if(status == 0)
      status = mspoolbdb_insert(mspool, fkey, fdata, (uint32_t)fdata_size);
      
    if(status == 0)
      log_info("Truncated mspoolbdb completely. Inserted %s.", fkey);
  }

  if(status != 0){
    if(status == ENOMEM)
      log_errx("Droping %s in mspoolbdb: %s", fkey, db_strerror(status));
    else
      log_errx("Error inserting %s in mspoolbdb: %s",
	       fkey, db_strerror(status));

    return(-1);
  }

  /* log_verbose(2, "nbsp_mspoolbdb_insert(): inserted %s", fkey); */
  
  return(0);
}

/*
 * slot read functions
 */
static int _nbsp_mspoolbdb_open(struct mspoolbdb_st *mspool, char *fpath){
  /*
   * Returns:
   *
   * -1 => error other than DB_NOTFOUND
   * -2 => not found in bdb (DB_NOTFOUND)
   * sd >= 0
   */
  int sd = -1;
  int status;
  char fkey[FKEY_SPOOLBDB_SIZE + 1];

  status = get_fpath_key(fpath, fkey);
  assert(status == 0);
  if(status != 0)
    return(-1);

  status = mspoolbdb_slots_open(mspool, fkey, &sd);
  if(status != 0){
    if(status == DB_NOTFOUND){
      log_errx("%s[%s] not found in mspoolbdb.", fpath, fkey);
      return(-2);
    }else{
      log_errx("Cannot open read slot in mspoolbdb for %s[%s]: %s",
	       fpath, fkey, db_strerror(status));
      return(-1);
    }
  }

  return(sd);
}

static int _nbsp_mspoolbdb_close(struct mspoolbdb_st *mspool, int sd){

  int status;

  status = mspoolbdb_slots_close(mspool, sd);
  if(status != 0){
    log_errx("Error closing read slot in mspoolbdb: %s", db_strerror(status));
    return(-1);
  }

  return(0);
}

static size_t _nbsp_mspoolbdb_read(struct mspoolbdb_st *mspool,
				   int sd, void *data, size_t data_size){
  size_t n;

  n = mspoolbdb_slots_read(mspool, sd, data, data_size);

  return(n);
}

static size_t _nbsp_mspoolbdb_datasize(struct mspoolbdb_st *mspool, int sd){

  return(mspoolbdb_slots_datasize(mspool, sd));
}

static int _nbsp_mspoolbdb_fpathsize(struct mspoolbdb_st *mspool,
				     char *fpath, size_t *size){
  int sd;

  sd = _nbsp_mspoolbdb_open(mspool, fpath);
  if(sd < 0)
    return(-1);

  *size = _nbsp_mspoolbdb_datasize(mspool, sd);
  (void)_nbsp_mspoolbdb_close(mspool, sd);

  return(0);
}

void nbsp_mspoolbdb_dbstats(void){

  DB_ENV *dbenv = NULL;

  if(g.mspoolbdb != NULL)
    dbenv = (g.mspoolbdb->spoolbdbv)->dbenv;
  else if(g.cspoolbdb != NULL)
    dbenv = (g.cspoolbdb->spoolbdbv)->dbenv;

  if(dbenv == NULL){
    /*
     * Neither memory nor cache spool configured.
     */
    return;
  }

  assert(g.mspoolbdb_dbstats_logfile != NULL);

  if(g.mspoolbdb_dbstats_logfile != NULL)
    nbsp_dbstats(dbenv, g.mspoolbdb_dbstats_logfile);
  else
    log_errx("Cannot write mspoolbdb dbstats: mspoolbdb_dbstats_logfile is NULL.");
}

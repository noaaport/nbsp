/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <db.h>
#include "dbstats.h"
#include "err.h"
#include "globals.h"
#include "dbenv.h"

#define DBENV_FLAGS	(DB_CREATE | DB_THREAD | DB_INIT_MPOOL)

int nbsp_open_dbenv(void){

  DB_ENV *dbenv = NULL;
  int status = 0;
  uint32_t mb;
  uint32_t dbenv_flags = DBENV_FLAGS;

  mb = (1024 * 1024) * g.dbcache_mb;

  status = db_env_create(&dbenv, 0);

  if(status == 0)
    status = dbenv->set_cachesize(dbenv, 0, mb, 0);

  if(status == 0)
    status = dbenv->open(dbenv, g.dbhome, dbenv_flags, g.dbfile_mode);

  if(status != 0){
    log_errx("Cannot initialize db environment. %s", db_strerror(status));
    status = -1;
    if(dbenv != NULL)
      (void)dbenv->close(dbenv, 0);
  }else
    g.dbenv = dbenv;

  return(status);
}

void nbsp_close_dbenv(void){

  int status;

  if(g.dbenv == NULL)
    return;

  if((status = g.dbenv->close(g.dbenv, 0)) != 0)
    log_errx("Error closing db environment. %s", db_strerror(status));
  else
    log_info("Closed db env.");

  g.dbenv = NULL;
}

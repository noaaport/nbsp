/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <err.h>
#include "dbenv.h"

#define DBENV_FLAGS	(DB_CREATE | DB_THREAD | DB_INIT_MPOOL)

static struct {
  int dbcache_mb;
  char *dbhome;
  mode_t dbfile_mode;
} g = {4, NULL, 0644};

DB_ENV *dbenv_open(void){

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
    warnx("Cannot initialize db environment. %s", db_strerror(status));
    if(dbenv != NULL){
      (void)dbenv->close(dbenv, 0);
      dbenv = NULL;
    }
  }

  return(dbenv);
}

void dbenv_close(DB_ENV *dbenv){

  int status;

  if(dbenv == NULL)
    return;

  if((status = dbenv->close(dbenv, 0)) != 0)
    warnx("Error closing db environment. %s", db_strerror(status));
  else
    fprintf(stdout, "%s\n", "Closed db env.");
}

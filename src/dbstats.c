/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <db.h>
#include "const.h"
#include "err.h"
#include "file.h"
#include "dbstats.h"

void nbsp_dbstats(DB_ENV *dbenv, char *logfile){
  /*
   * DB_STAT_ALL | DB_STAT_SUBSYSTEM
   */
  int status;
  FILE *fp;
  char *tmplogfile;

  tmplogfile = make_temp_logfile(logfile, TEMP_FILE_EXT);
  if(tmplogfile == NULL)
    log_err("Error reporting db stats.");

  fp = fopen(tmplogfile, "w");
  if(fp == NULL){
    log_err2("Cannot open", tmplogfile);
    free(tmplogfile);
    return;
  }
  
  dbenv->set_msgfile(dbenv, fp);
  status = dbenv->stat_print(dbenv, DB_STAT_SUBSYSTEM);
  if(status != 0)
    log_errx("Error reporting db stats: %s", db_strerror(status));

  (void)dbenv->set_msgfile(dbenv, NULL);
  fclose(fp);

  if(status == 0){
    status = rename(tmplogfile, logfile);
    if(status != 0)
      log_err2("Error renaming", tmplogfile);
  }else
    (void)unlink(tmplogfile);

  free(tmplogfile);
}

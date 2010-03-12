/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include "err.h"
#include "file.h"
#include "const.h"
#include "globals.h"
#include "spooldb.h"
#include "espooldb.h"

/*
 * The higher level fs spooldb functions (see espooldb.h)
 */
int e_spooldb_open(void){

  int keysize;
  int status = 0;

  g.spooldb = NULL;

  if(g.spooldb_slots == 0){
    /*
     * The spooldb bookeeping is disabled. The spool directory will continue
     * to grow, and the hourly cron script must handle the cleaning job
     * of the spool directory.
     */
    return(0);
  }

  if(g.spooldb_slots < SPOOLDB_SLOTS_MIN)
    g.spooldb_slots = SPOOLDB_SLOTS_MIN;

  keysize = const_fpath_maxsize() + 1;

  g.spooldb = spooldb_init(g.spooldb_slots, keysize,
			   e_spooldb_delete_fpath);
  if(g.spooldb == NULL){
    log_err("Could not create spooldb.");
    return(-1);
  }

  status = file_exists(g.spooldb_fname);
  if(status == 1){
    /*
     * The file does not exist.
     */
    return(0);
  }

  if(status == 0)
    status = spooldb_read(g.spooldb, g.spooldb_fname);

  if(status == -1)
    log_err_read(g.spooldb_fname);
  else if(status == 1)
    log_errx("Saved spooldb file is inconsistent with configuration.");

  if(status != 0){
    spooldb_destroy(g.spooldb);
    g.spooldb = NULL;
  }

  return(status);
}

void e_spooldb_close(void){

  int status = 0;

  if(g.spooldb == NULL){
    /*
     * The spooldb was not enabled.
     */
    return;
  }

  status = spooldb_write(g.spooldb, g.spooldb_fname, g.dbfile_mode);
  if(status != 0)
    log_err_write(g.spooldb_fname);

  spooldb_destroy(g.spooldb);
  g.spooldb = NULL;
}

int e_spooldb_insert(char *fpath){

  int status = 0;

  if(g.spooldb == NULL){
    /*
     * The spooldb was not enabled.
     */
    return(0);
  }

  status = spooldb_insert(g.spooldb, fpath);
  if(status != 0){
    /*
     * An error here means that the file occupying the slot where the
     * new key was inserted could not be deleted.
     */
    log_err("Could not delete old file from spool directory.");
  }

  return(status);
}

int e_spooldb_delete_fpath(char *fpath){

  int status = 0;

  if(fpath[0] == '\0'){
    /*
     * Unoccupied slot.
     */
    return(0);
  }

  status = file_delete(fpath);
  if(status != 0)
    log_err2("Could not delete", fpath);

  return(status);
}

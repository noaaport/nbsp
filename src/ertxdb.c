/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <string.h>
#include "err.h"
#include "globals.h"
#include "rtxdb.h"

/*
 * In this version of the rtxdb, the numslots parameter is not used.
 * That parameter is used here only to determine if the rtxdb
 * mechanism should be disabled. If numslots is set to zero,
 * it will be disabled. (To keep the database from growing unmanageably large,
 * it is truncated periodically).
 */
int e_nbsp_rtxdb_open(void){

  int status;

  if(g.rtxdb_slots == 0)
    return(0);

  status = nbsp_rtxdb_open(&g.rtxdb, g.dbenv, g.rtxdb_dbfname, g.dbfile_mode);

  if(status != 0)
    status = nbsp_rtxdb_log_err("Could not open rtx db.", status);

  return(status);
}

int e_nbsp_rtxdb_close(void){

  int status;

  if(g.rtxdb == NULL)
    return(0);

  status = nbsp_rtxdb_close(g.rtxdb);
  if(status != 0)
    status = nbsp_rtxdb_log_err("Error closing rtx db.", status);
  else
    log_info("Closed rtx db.");

  return(status);
}

int e_nbsp_rtxdb_truncate(void){

  int status;

  if(g.rtxdb == NULL)
    return(0);

  status = nbsp_rtxdb_truncate(g.rtxdb);
  if(status != 0)
    status = nbsp_rtxdb_log_err("Error truncating rtx db.", status);
  else
    log_info("Truncated rtx db.");

  return(status);
}

int e_nbsp_rtxdb_put_ok(unsigned int seqnum){

  int status;
  int fstatus = 0;	/* ok */

  if(g.rtxdb == NULL){
    /*
     * rtx db was not enabled.
     */
    return(0);
  }

  status = nbsp_rtxdb_put(g.rtxdb, seqnum, fstatus);
  if(status != 0)
    status = nbsp_rtxdb_log_err("Could not put record in rtx db.", status);

  return(status);
}

int e_nbsp_rtxdb_put_fail(unsigned int seqnum){

  int status;
  int fstatus = 1;	/* fail */

  if(g.rtxdb == NULL){
    /*
     * rtx db was not enabled.
     */
    return(0);
  }

  status = nbsp_rtxdb_put(g.rtxdb, seqnum, fstatus);
  if(status != 0)
    status = nbsp_rtxdb_log_err("Could not put record in rtx db.", status);

  return(status);
}

int e_nbsp_rtxdb_get_fstatus(unsigned int seqnum){
  /*
   * This function returns:
   *
   * 0   seqnumber found with ok (0) status
   * 1   seqnumber found with fail (1) seqnumber
   *
   * or, an error as follows:
   * 
   * -1  if there is an error trying to get the record (including seqnum
   *     not found or some other db error).
   * -2  rtx db mechanism not enabled.
   */
  int status = 0;
  int fstatus;

  if(g.rtxdb == NULL){
    /*
     * nbspdb was not enabled.
     */
    return(-2);
  }

  status = nbsp_rtxdb_get(g.rtxdb, seqnum, &fstatus);
  if(status != 0){
    if(status == DB_NOTFOUND)
      log_errx("%u not found in rtx nbspdb.", seqnum);
    else
      (void)nbsp_rtxdb_log_err("Could not get record from rtx db.", status);

    return(-1);
  }

  status = fstatus;

  return(status);
}

int nbsp_rtxdb_log_err(char *s, int status){

  assert(status != 0);

  log_errx("%s %s", s, db_strerror(status));

  if(status > 0)
    status = -1;
  else if(status < 0)
    status = 1;

  return(status);
}

/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <string.h>
#include "err.h"
#include "globals.h"
#include "nbspdb.h"

/*
 * If the configuration defined numslots = 0, then the nbspdb
 * is disabled. These functions return an appropriate code
 * but do nothing.
 */

int e_nbspdb_open(void){

  int status;

  if(g.nbspdb_slots == 0){
    /*
     * The retransmission bookeeping is disabled. 
     */
    return(0);
  }

  if(g.nbspdb_slots < NBSPDB_SLOTS_MIN)
    g.nbspdb_slots = NBSPDB_SLOTS_MIN;

  status = nbspdb_open(&g.nbspdb, g.dbenv,
		       g.nbspdb_slots, g.nbsp_dbfname, g.dbfile_mode);

  if(status != 0)
    status = nbspdb_log_err("Could not open spool db.", status);

  return(status);
}

int e_nbspdb_close(void){

  int status;

  if(g.nbspdb == NULL)
    return(0);

  status = nbspdb_close(g.nbspdb);
  if(status != 0)
    status = nbspdb_log_err("Error closing spool db.", status);
  else
    log_info("Closed spool db.");

  return(status);
}

int e_nbspdb_put_ok(unsigned int seqnum){

  int status;

  if(g.nbspdb == NULL){
    /*
     * Not enabled.
     */
    return(0);
  }

  status = nbspdb_put_ok(g.nbspdb, seqnum);
  if(status != 0)
    status = nbspdb_log_err("Could not put record in spool db.", status);

  return(status);
}

int e_nbspdb_put_fail(unsigned int seqnum){

  int status;

  if(g.nbspdb == NULL){
    /*
     * Not enabled.
     */
    return(0);
  }

  status = nbspdb_put_fail(g.nbspdb, seqnum);
  if(status != 0)
    status = nbspdb_log_err("Could not put record in spool db.", status);

  return(status);
}

int e_nbspdb_get_status(unsigned int seqnum){
  /*
   * This function returns:
   *
   * 0   seqnumber matched and status is 0
   * 1   seqnumber matched and status is 1
   * 
   * or, an error as follows:
   * 
   * -1  if there is an error trying to get the record (including seqnumber
   *     not found or some other db error).
   *
   * -2  the db mechanism was not enabled. 
   */
  int status = 0;
  struct nbspdb_data_st data;

  if(g.nbspdb == NULL){
    /*
     * Not enabled.
     */
    return(-2);
  }

  data.seqnum = seqnum;
  status = nbspdb_get(g.nbspdb, &data);
  if(status != 0){
    status = nbspdb_log_err("Could not get record from spool db.", status);
    return(-1);
  }

  status = data.status;

  return(status);
}

int nbspdb_log_err(char *s, int status){

  assert(status != 0);

  log_errx("%s %s", s, db_strerror(status));

  if(status > 0)
    status = -1;
  else if(status < 0)
    status = 1;

  return(status);
}

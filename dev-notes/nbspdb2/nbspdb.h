/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef NBSPDB_H
#define NBSPDB_H

#include <db.h>
#include "const.h"

/*
 * The purpose of this db is to keep track of products received with missing
 * frames (incomplete products). Only the processor reads and writes this db.
 *
 * But this database does not keep track of whether the file was sucessfully
 * saved to disk, or whether the file is in the postprocessing queue
 * (filters and server's queue) or whether they have already
 * been postprocessed, and so on.
 *
 * The main use for this database is to be able to recognize if a
 * retrasmission of a file should be processed or whether it can be ignored,
 * and whether a previously missed product is being recovered in the
 * retransmission.
 * If the status is not zero the retransmision should be processed.
 *
 * All these functions return the same error codes as the db functions:
 * either 0, errno or some negative number indicating a db specific error.
 *
 * The e_xxx_xxx() functions differ from the xxx_xxx() ones as follows:
 *
 * (1) They write an error if there is one.
 * (2) They return -1 for a system error or 1 for a db specific error.
 *
 * The functions are used only by the processor (nbsp.c) and only
 * via the higher level (e_xxx) functions.
 */

/*
 * In each slot we put the the sequence number of the product received,
 * and its status (0 => ok, 1 => fail).
 */
struct nbspdb_data_st {
  unsigned int seqnum;
  int status;
};

struct nbspdb_st {
  DB *dbp;
  DB *sdbp;
  unsigned int numslots;	/* number of occupied slots */
  unsigned int numslots_max;
};

int nbspdb_open(struct nbspdb_st **nbspdb, DB_ENV *dbenv,
		unsigned int numslots, char *dbfname, int mode);
int nbspdb_close(struct nbspdb_st *nbspdb);
int nbspdb_put_ok(struct nbspdb_st *nbspdb, unsigned int seqnum); 
int nbspdb_put_fail(struct nbspdb_st *nbspdb, unsigned int seqnum);
int nbspdb_get(struct nbspdb_st *nbspdb, struct nbspdb_data_st *data);

/*
 * This version of the functions are in the file enbspdb.c
 */
int e_nbspdb_open(void);
int e_nbspdb_close(void);
int e_nbspdb_put_ok(unsigned int seqnum);
int e_nbspdb_put_fail(unsigned int seqnum);
int e_nbspdb_get_status(unsigned int seqnum);
int nbspdb_log_err(char *s, int status);

#endif

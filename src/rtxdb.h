/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef RTXDB_H
#define RTXDB_H

#include <db.h>

/*
 * The purpose of this db is to keep track of products received for the purpose
 * of handling retransmissions. Only the processor reads and writes this db.
 *
 * This database does not keep track of whether the file was sucessfully
 * saved to disk, or whether the file is in the postprocessing queue
 * (filters and server's queue) or whether they have already
 * been postprocessed, and so on.
 *
 * The main use for this database is to be able to recognize if a
 * retrasmission of a file should be processed or whether it can be ignored,
 * and whether a previously missed product is being recovered in the
 * retransmission.  If the status is not zero the retransmision
 * should be processed.
 *
 * At 1,500 products/minute (90,000/hour) the databases can grow very
 * large. The lookups in periods of too many retransmissions can then
 * take significant resources. We use can two approaches: (i) limit the
 * size of the database (e.g., using a circular array) or (ii) truncate
 * the databases periodically. We use the latter; it is a rather heuristic
 * approach, but it avoids introducing more overhead, and the retransmission
 * handling logic is all heuristic anyway. The function nbspdb_truncate()
 * is for that.
 *
 * The key is the seqnum and the data is the status, which indicates if
 * the file was received correctly or not.
 */

/*
 * All these functions return the same error codes as the db functions:
 * either 0, errno or some negative number indicating a db specific error.
 *
 * The e_xxx_xxx() functions differ from the xxx_xxx() ones as follows:
 *
 * (1) They write an error if there is one.
 * (2) They return -1 for a system error or 1 for a db specific error.
 *
 * The functions are used only by the processor (nbsp.c) and only
 * via the higher level (e_xxx) functions in enbspdb.c.
 */

struct nbsp_rtxdb_st {
  DB *dbp_c;	/* current */
  DB *dbp_o;	/* previous */
  DB *dbp1;
  DB *dbp2;
};

struct nbsp_rtxdb_data_st {
  int fstatus;
};

struct nbsp_rtxdb_rec_st {
  unsigned int seqnum;
  struct nbsp_rtxdb_data_st data;
};

int nbsp_rtxdb_open(struct nbsp_rtxdb_st **rtxdb,
		    DB_ENV *dbenv, char *dbfname, mode_t mode);
int nbsp_rtxdb_close(struct nbsp_rtxdb_st *rtxdb);
int nbsp_rtxdb_put(struct nbsp_rtxdb_st *rtxdb,
		   unsigned int seqnum, int fstatus);
int nbsp_rtxdb_get(struct nbsp_rtxdb_st *rtxdb,
		   unsigned int seqnum, int *fstatus);
int nbsp_rtxdb_truncate(struct nbsp_rtxdb_st *rtxdb);

/*
 * This version of the functions are in the file enbspdb.c
 */
int e_nbsp_rtxdb_open(void);
int e_nbsp_rtxdb_close(void);
int e_nbsp_rtxdb_truncate(void);
int e_nbsp_rtxdb_put_ok(unsigned int seqnum);
int e_nbsp_rtxdb_put_fail(unsigned int seqnum);
int e_nbsp_rtxdb_get_fstatus(unsigned int seqnumber);
int nbsp_rtxdb_log_err(char *s, int status);

#endif

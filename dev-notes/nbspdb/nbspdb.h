/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef NBSPDB_H
#define NBSPDB_H

#include <db.h>
/* #include "const.h" */
#define FNAME_SIZE	18

/*
 * This database keeps track of the files as they are received and saved to
 * disk. The status variable starts with the value 1, when the first frame
 * is received and the product becomes the active element in the pctl.
 * When the last frame is received the status variable is set to 2.
 * Finally, when the processor processes the element and saves the file
 * to disk the variable is set to 0. Therefore, in the end the values mean
 * the following:
 * 0 indicates that the file was saved without errors; 
 * 1 if there was some reception error (e.g., incomplete product;
 * not all frames received);
 * 2 if there were errors processing the file (memory, no disk space,
 * bad frames, etc). 
 * But this database does not keep track of
 * whether the file is in the postprocessing queue (filters and server's queue)
 * or whether they have already been postprocessed, and so on.
 *
 * The main use for this database is to be able to recognize if a
 * retrasmission of a file should be processed or whether it can be ignored.
 * If the status is not zero the retransmision should be processed,
 * but it can be ignored otherwise.
 *
 * All these functions return the same error codes as the db functions:
 * either 0, errno or some negative number indicating a db specific error.
 *
 * Since these functions are used by the readers (when a new product 
 * is started) and by the processor (when the product is saved),
 * they must be called protected by a mutex since they are all different
 * threads. That is the reason of the e_xxx_xxx() functions,
 * which differ as follows:
 *
 * (1) They are called proteted by a mutex.
 * (2) They write an error if there is one.
 * (3) They return -1 for a system error or 1 for a db spscific error.
 *
 * The xxx_xxx_lock() functions also protect the calls to the basic functions
 * with the mutex, but do not print the error message. Their return value
 * is the same as the basic functions.
 */

#define NBSPDB_STATUS_INCOMPLETE	1
#define NBSPDB_STATUS_UNPROCESSED	2

struct nbspdb_pdata_st{
  unsigned int seq;
  int status;
};

struct nbspdb_prec_st{
  char fname[FNAME_SIZE + 1];
  struct nbspdb_pdata_st data;
};

struct nbspdb_st{
  DB *dbp;	/* primary */
  DB *sdbp;	/* secondary */
  DBC *cursor;
};

int nbspdb_open(struct nbspdb_st **nbspdb, char *dbfname, char *sdbfname,
		u_int32_t dbflags);
int nbspdb_close(struct nbspdb_st *nbspdb);
int nbspdb_put(struct nbspdb_st *nbspdb, struct nbspdb_prec_st *prec);
int nbspdb_get_byname(struct nbspdb_st *nbspdb, struct nbspdb_prec_st *prec);
int nbspdb_get_byseq(struct nbspdb_st *nbspdb, struct nbspdb_prec_st *prec);
int nbspdb_update(struct nbspdb_st *nbspdb, struct nbspdb_prec_st *prec);
int nbspdb_put2(struct nbspdb_st *nbspdb,
		char *fname, unsigned int seqnum, int status);
int nbspdb_update2(struct nbspdb_st *nbspdb,
		   char *fname, unsigned int seqnum, int status);

/*
 * This version of the functions are in the file enbspdb.c
 */
int e_nbspdb_open(void);
int e_nbspdb_close(void);
int e_nbspdb_put2(char *fname, unsigned int seqnum, int status);
int e_nbspdb_get_byname(struct nbspdb_prec_st *prec);
int e_nbspdb_get_byseq(struct nbspdb_prec_st *prec);
int e_nbspdb_update2(char *fname, unsigned int seqnum, int status);
int e_nbspdb_updateput(char *fname, unsigned int seqnum, int fstatus);
int e_nbspdb_update_status(char *fname, unsigned int seqnum, int fstatus);
int nbspdb_log_err(char *s, int status);

/*
 * The xxx_lock() version of the functions, lock the db, but do not
 * print an error message.
 */
int nbspdb_update_status_lock(char *fname, unsigned int seqnum, int fstatus);

#endif

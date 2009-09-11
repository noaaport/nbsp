/*
 * Copyright (c) 2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SPOOLDB_H
#define SPOOLDB_H

/*
 * In the present version of nbsp, the raw data files are kept in the
 * file system itself, in the spool directory, and not in a database.
 * By means of the functions of this file, the names (full path) of those
 * files are kept in a circular array (of a configurable size). When a new
 * file fills an occupied slot in the array, the program deletes from the
 * spool directory the file that was in that slot. In this way the number 
 * of files in the spool directory is kept at the specified number
 * and there is no need to include the spool directory in the configuration
 * file of the hourly (diretory cleanup) cron script.
 *
 * These functions are used only by the processor (nbsp.c) and are called
 * only via the higher level functions defined there.
 */

/*
 * The data in each slot will be the full path to the file.
 * The program must pass to the library a function that deletes the
 * file.
 */
typedef int (*destroy_key_proc)(char *key);

struct spooldb_st {
  unsigned int numslots;
  int	       keysize;		/* strlength(fpath) + 1 */
  unsigned int slot;		/* next available slot */
  unsigned int index;		/* start pos of next slot in keys buffer */
  char         *slotptr;	/* start of next slot in keys buffer */
  char	       *keys;		/* (numslots * keysize) */
  destroy_key_proc destroy_key;  
};

struct spooldb_st *spooldb_init(unsigned int numslots, int keysize,
				destroy_key_proc destroy_key);
void spooldb_destroy(struct spooldb_st *spooldb);
int spooldb_insert(struct spooldb_st *spooldb, char *key);
int spooldb_write(struct spooldb_st *spooldb, char *filename, mode_t mode);
int spooldb_read(struct spooldb_st *spooldb, char *filename);
unsigned int spooldb_get_slot(struct spooldb_st *spooldb);

#endif

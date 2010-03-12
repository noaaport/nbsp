/*
 * Copyright (c) 2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef ESPOOLDB_H
#define ESPOOLDB_H

/*
 * The higher level interface to the spooldb (spool bookeeping db) library.
 * It is used by all the feeds that use the spool: the processor, the
 * infeed and the slave nbs2 feed. The nbsq feed uses the processor and
 * therefore does not use the spool directly itself.
 */
int e_spooldb_open(void);
void e_spooldb_close(void);
int e_spooldb_insert(char *fpath);
int e_spooldb_delete_fpath(char *fpath);

#endif

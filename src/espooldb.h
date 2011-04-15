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
 * The spooldb functions are used only in nbsp.c and these espooldb functions
 * are not being used.
 */
int e_spooldb_open(void);
void e_spooldb_close(void);
int e_spooldb_insert(char *fpath);
int e_spooldb_delete_fpath(char *fpath);

#endif

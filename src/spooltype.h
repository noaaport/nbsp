/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SPOOLTYPE_H
#define SPOOLTYPE_H

int spooltype_fsspool(void);  /* file system */
int spooltype_mspool(void);   /* memory based private spool */
int spooltype_cspool(void);   /* shared bdb with cache (fs backed or mmaped) */
int spooltype_cspool_nofile(void); /* mmaped cspool  */

#endif

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
int spooltype_mspool(void);   /* in-memory private spool (fs or mem dbenv) */
int spooltype_mspool_nodbhome(void);   /* mem dbenv; pure in-memory db */
int spooltype_cspool(void);   /* shared bdb with cache (fs backed or mmaped) */
int spooltype_cspool_nofile(void); /* mmaped cspool  */

#endif

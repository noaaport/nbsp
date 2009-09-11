/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef NBSP_H
#define NBSP_H

#include "sbn.h"

int init_pctl(void);
void kill_pctl(void);
int spawn_nbsproc(void);
void kill_processor_thread(void);

/* to be called periodically */
void sync_pctl(void);
void rept_pctl_quota(void);
void set_rtxdb_truncate_flag(void);

#endif

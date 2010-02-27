/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef NBSP_DBPANIC_H
#define NBSP_DBPANIC_H

void nbsp_bdbpanic(void);
void nbsp_db_event_callback(DB_ENV *dbenv __attribute__ ((unused)),
                            uint32_t event,
                            void *event_info __attribute__ ((unused)));
#endif

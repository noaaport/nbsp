/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef INIT_H
#define INIT_H

void init_globals(void);
void cleanup(void);
void cleanup_files(void);
int init_daemon(void);
int init_lock(void);
int init_directories(void);
int drop_privs(void);
int init_feeds(void);
int init_queues(void);
int init_mspool(void);
int spawn_feeds(void);
int spawn_processor(void);

#endif

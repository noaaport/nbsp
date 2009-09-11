/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SLAVE_H
#define SLAVE_H

/* 
 * servermode 0 is master mode, defined in defaults.h
 */
#define FEEDMODE_SLAVENET_NBS1     1    /* send file content */
#define FEEDMODE_SLAVENET_NBS2     2    /* send file fpath */
#define FEEDMODE_SLAVEINFEED       3    /* via the infeed fifo */
#define FEEDMODE_MASTER_INFEED     4    /* master plus the infeed fifo */

int slave_open(void);
int slave_close(void);
int slave_reopen(void);
int spawn_slave(void);
void kill_slave_thread(void);
int feedmode_slavenet_nbs1_enabled(void);
int feedmode_slavenet_nbs2_enabled(void);
int feedmode_slaveinfeed_enabled(void);
int feedmode_master_enabled(void);
int feedmode_slave_enabled(void);

/* This function is used by each slave thread module. */
int spawn_slave_thread(void *(*thread_main)(void*));

#endif

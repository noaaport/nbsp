/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SLAVE_H
#define SLAVE_H

int feedmode_noaaport_enabled(void);
int feedmode_masterservers_enabled(void);
int feedmode_inputfifo_enabled(void);
int feedmode_slave_enabled(void);
int feedmode_slave_nbs1_enabled(void);

int init_slavet(void);
void cleanup_slavet(void);

int spawn_slave_threads(void);
void kill_slave_threads(void);

#endif

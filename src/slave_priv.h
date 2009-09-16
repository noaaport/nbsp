/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SLAVE_PRIV_H
#define SLAVE_PRIV_H

/*
 * These are (initialization) functions defined in slave_net.c and slave_in.c
 * that are used in slave.c. This file is included only in slave.c and those
 * two files.
 */

/* slave_net.c */
int slavenet_spawn_thread(struct slave_element_st *slave);
void slavenet_kill_thread(struct slave_element_st *slave);

/* slave_in.c */
int slavein_spawn_thread(struct slave_element_st *slave);
void slavein_kill_thread(struct slave_element_st *slave);

#endif

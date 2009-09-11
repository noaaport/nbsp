/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SERVER_H
#define SERVER_H

int spawn_server(void);
void kill_server_thread(void);
void set_reload_server_filters_flag(void);
void set_server_state_flag(void);

#endif

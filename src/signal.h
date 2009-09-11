/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SIGNAL_H
#define SIGNAL_H

int init_signals(void);
int init_signals_block(void);
int init_signals_thread(void);
int get_quit_flag(void);
int get_hup_flag(void);
int get_alarm_flag(void);
void set_quit_flag(void);

#endif

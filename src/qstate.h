/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef QSTATE_H
#define QSTATE_H

int init_qstate_fifo(void);
void kill_qstate_fifo(void);
void e_report_qstate(void);
void set_reopen_qstatefifo_flag(void);
void log_qstate(void);

#endif


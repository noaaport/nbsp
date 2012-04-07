/*
 * Copyright (c) 2002-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef LIBCONNTH_POLL_H
#define LIBCONNTH_POLL_H

#include "conn.h"
#include "tcpsock.h"

/*
 * These functions return 0 or an error code. If the error
 * originated in a db function, then ct->dberror will have the db error.
 * If ct->dberror is zero and the function returned an error, then
 * the error came from a system call or function.
 */
int poll_loop(struct conn_table_st *ct,
	      struct client_options_st *clientopts);
int poll_loop_nowait(struct conn_table_st *ct,
		     struct client_options_st *clientopts);
int poll_loop_wait(struct conn_table_st *ct, int timeout,
		   struct client_options_st *clientopts);
int poll_kill_client_connection(struct conn_table_st *ct, int client_index);

#endif

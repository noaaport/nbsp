/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SERVER_PRIV_H
#define SERVER_PRIV_H

#include "libconnth/libconn.h"
#include "packfp.h"
#include "sfilter.h"

/*
 * These functions are defined in serverc.c and used by the main
 * thread (serverm.c).
 */
int client_thread_create(struct conn_element_st *ce, pthread_t *t_id);
int client_thread_kill(struct conn_element_st *ce);

/*
 * Defined in serverf.c and used in serverm.c
 */
int exec_net_filter(struct sfilterp_st *sfilterp, struct conn_table_st *ct);
int allow_filter_init(struct conn_element_st *ce);
int allow_filter_get_flag(struct conn_element_st *ce);

#endif

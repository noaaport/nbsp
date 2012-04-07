/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef LIBCONNTH_CE_H
#define LIBCONNTH_CE_H

#include "conn.h"

int conn_element_init1(struct conn_element_st *ce,
		       int fd,
		       int type, pid_t pid, char *ip, char *name,
		       int write_timeout_ms,
		       int write_timeout_retry,
		       int reconnect_wait_sleep_secs,
		       int reconnect_wait_sleep_retry,
		       int queue_read_timeout_ms);
int conn_element_init2(struct conn_element_st *ce,
		       void *appdata, free_appdata_proc freedataproc);
int conn_element_init3(struct conn_element_st *ce, void *filterdata,
		       free_filterdata_proc freefilterdataproc);
int conn_element_init4(struct conn_element_st *ce,
		       thread_create_proc thcreateproc,
		       thread_kill_proc thkillproc,
		       struct connqueue_param_st *cqparam, int *dberror);
int conn_element_release(struct conn_element_st *ce, int *dberror);
int conn_element_isclient(struct conn_element_st *ce);
int conn_element_isnetclient(struct conn_element_st *ce);
int conn_element_isnetclient_running(struct conn_element_st *ce);
int conn_element_isnetclient_stoped(struct conn_element_st *ce);
int conn_element_isserver(struct conn_element_st *ce);

int conn_element_get_fd(struct conn_element_st *ce);
void conn_element_set_fd(struct conn_element_st *ce, int fd);
int conn_element_get_pid(struct conn_element_st *ce);
int conn_element_get_type(struct conn_element_st *ce);
char *conn_element_get_ip(struct conn_element_st *ce);
char *conn_element_get_name(struct conn_element_st *ce);
char *conn_element_get_nameorip(struct conn_element_st *ce);
void *conn_element_get_appdata(struct conn_element_st *ce);
void *conn_element_get_filterdata(struct conn_element_st *ce);
connqueue_t *conn_element_get_queue(struct conn_element_st *ce);
struct conn_stats_st *conn_element_get_cstats(struct conn_element_st *ce);
int conn_element_report_cstats(struct conn_element_st *ce,
			       int count, time_t period, char *fname);

time_t conn_element_get_ctime(struct conn_element_st *ce);

int conn_element_get_created_flag(struct conn_element_st *ce);
int conn_element_get_finished_flag(struct conn_element_st *ce);
int conn_element_set_finished_flag(struct conn_element_st *ce);

int conn_element_get_connection_status(struct conn_element_st *ce,
				       int *fd);
int conn_element_set_connection_status(struct conn_element_st *ce,
				       int fd);
int conn_element_get_connection_status_un(struct conn_element_st *ce);

int conn_element_get_exit_flag(struct conn_element_st *ce);
void conn_element_set_exit_flag(struct conn_element_st *ce);

int conn_element_get_write_timeout_ms(struct conn_element_st *ce);
void conn_element_set_write_timeout_ms(struct conn_element_st *ce,
				       int write_timeout_ms);

int conn_element_get_write_timeout_retry(struct conn_element_st *ce);
void conn_element_set_write_timeout_retry(struct conn_element_st *ce,
					  int write_timeout_retry);

int conn_element_get_reconnect_wait_sleep_secs(struct conn_element_st *ce);
void conn_element_set_reconnect_wait_sleep_secs(struct conn_element_st *ce,
					int reconnect_wait_sleep_secs);

int conn_element_get_reconnect_wait_sleep_retry(struct conn_element_st *ce);
void conn_element_set_reconnect_wait_sleep_retry(struct conn_element_st *ce,
					int reconnect_wait_sleep_retry);

int conn_element_get_queue_read_timeout_ms(struct conn_element_st *ce);
void conn_element_set_queue_read_timeout_ms(struct conn_element_st *ce,
					    int queue_read_timeout_ms);
#endif

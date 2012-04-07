/*
 * Copyright (c) 2002-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef LIBCONNTH_CONN_H
#define LIBCONNTH_CONN_H

#include <sys/types.h>
#include <pthread.h>
#include <poll.h>
#include "cq.h"
#include "cs.h"

/*
 * Forward structure declarations, so we can declare pointers.
 */
struct conn_element_st;
struct conn_table_st;

/*
 * This is a pointer to a function that will handle the requests from
 * clients when poll() detects that a client is sending us a message.
 * These are POLLIN messages and such that a read() will not return
 * 0 or -1.
 */
typedef int (*conn_handler_proc)(struct conn_table_st *ct, int i);

/*
 * This function is called to notify the application
 * when poll_loop() detects that the client
 * has closed, and it is about to delete this element from the table.
 * These are POLLHUP and ERR situations, or POLLIN with a 0 or -1 read
 * condition. The last argument "cond" indicates which condition ocurred.
 * (See poll_client_hangup() in poll.c.)
 */
typedef void (*hangup_handler_proc)(struct conn_table_st *ct, int i, int cond);

/*
 * For tcpwrappers or equivalent functionality.
 */
typedef int (*client_access_proc)(int fd, char *ip, char *name);

/*
 * Functions to create and destroy the threads when a new client connects.
 */
typedef int (*thread_create_proc)(struct conn_element_st *ce, pthread_t *tid);
typedef int (*thread_kill_proc)(struct conn_element_st *ce);

/*
 * To free apdata
 */
typedef void (*free_appdata_proc)(void *data);

/*
 * To free the filter structure
 */
typedef void (*free_filterdata_proc)(void *filterdata);

/*
 * The thread information and connection status for each client thread.
 * The id and the created flag are used by the server main thread only.
 */
struct thread_info_st {
  pthread_t thread_id;
  int f_thread_created;
  int f_thread_finished;	/* shared between main and client thread */
  int connection_status;	/* shared between main and client thread */
  int client_fd;		/* shared between main and client thread */
  pthread_mutex_t mutex;
};

struct conn_element_st {
  struct conn_stats_st cs;	/* connection statistics */
#define CONN_TYPE_NONE		0
#define CONN_TYPE_SERVER_LOCAL	1
#define CONN_TYPE_SERVER_NET	2
#define CONN_TYPE_CLIENT_LOCAL	3
#define CONN_TYPE_CLIENT_NET	4
#define CONN_TYPE_APPLICATION	5	/* application defined */
  int type;
  pid_t pid;
  char *ip;
  char *name;			     /* copy of hostent->h_name of client */
  int fd;
  int write_timeout_ms;
  int write_timeout_retry;
  int reconnect_wait_sleep_secs;
  int reconnect_wait_sleep_retry;
  int queue_read_timeout_ms;
  void *appdata;		     /* for application use (e.g. protocol) */
  free_appdata_proc freedataproc;
  void *filterdata;		     /* filter data to support access */
  free_filterdata_proc freefilterdataproc;
  thread_kill_proc thkillproc;
  connqueue_t *cq;		     /* a libqdb table of only one entry */
  struct thread_info_st *threadinfo;
  int f_exit;			     /* used by the client thread itself */
};

struct conn_table_st {
  int n;		/* number of used entries */
  int nmax;		/* current size of the arrays */
  int nclients;		/* number of network client connections */
  int nclientsmax;	/* max number of network client connections */
  struct pollfd *pfd;
  struct conn_element_st *ce;	/* an array of conn elements */
  conn_handler_proc connproc;
  hangup_handler_proc hangupproc;
  client_access_proc accessproc;
  pthread_mutex_t mutex;
  int status;
  int dberror;
};

struct conn_table_st *conn_table_create(int maxclients,
					conn_handler_proc connproc,
					hangup_handler_proc hangupproc,
					client_access_proc accessproc);
void conn_table_destroy(struct conn_table_st *ct);

/* the mutex locking functions are at the end */
int conn_table_add_element_un(struct conn_table_st *ct,
			      int fd,
			      int type, pid_t pid, char *ip,char *name,
			      int write_timeout_ms,
			      int write_timeout_retry,
			      int reconnect_wait_sleep_secs,
			      int reconnect_wait_sleep_retry,
			      int queue_read_timeout_ms);
int conn_table_del_element_un(struct conn_table_st *ct, int i);

int conn_table_get_element_fd(struct conn_table_st *ct, int i);
int conn_table_get_element_pid(struct conn_table_st *ct, int i);
int conn_table_get_element_type(struct conn_table_st *ct, int i);
char *conn_table_get_element_ip(struct conn_table_st *ct, int i);
char *conn_table_get_element_name(struct conn_table_st *ct, int i);
char *conn_table_get_element_nameorip(struct conn_table_st *ct, int i);
int conn_table_element_isnetclient(struct conn_table_st *ct, int i);
int conn_table_element_isnetclient_running(struct conn_table_st *ct, int i);
int conn_table_element_isnetclient_stoped(struct conn_table_st *ct, int i);
time_t conn_table_get_element_ctime(struct conn_table_st *ct, int i);
int conn_table_find_element_byip(struct conn_table_st *ct, char *ip);

void conn_table_set_element_appdata(struct conn_table_st *ct, int i,
				   void *data, free_appdata_proc freedataproc);
void conn_table_set_element_filterdata(struct conn_table_st *ct, int i,
				       void *filterdata,
				      free_filterdata_proc freefilterdataproc);
void *conn_table_get_element_appdata(struct conn_table_st *ct, int i);
void *conn_table_get_element_filterdata(struct conn_table_st *ct, int i);

int conn_table_set_element_thread(struct conn_table_st *ct, int i,
				  thread_create_proc thcreateproc,
				  thread_kill_proc thkillproc,
				  struct connqueue_param_st *cqparam);
connqueue_t *conn_table_get_element_queue(struct conn_table_st *ct, int i);
int conn_table_get_element_fthread_created(struct conn_table_st *ct, int i);
int conn_table_get_element_fthread_finished(struct conn_table_st *ct, int i);

int conn_table_get_numentries(struct conn_table_st *ct);
int conn_table_get_netclients(struct conn_table_st *ct);
int conn_table_get_netclientsmax(struct conn_table_st *ct);

/* mutex locking functions */
int conn_table_add_element(struct conn_table_st *ct,
			   int fd,
			   int type, pid_t pid, char *ip, char *name,
			   int write_timeout_ms,
			   int write_timeout_retry,
			   int reconnect_wait_sleep_secs,
			   int reconnect_wait_sleep_retry,
			   int queue_read_timeout_ms);
int conn_table_del_element(struct conn_table_st *ct, int i);

int conn_table_mutex_lock(struct conn_table_st *ct);
int conn_table_mutex_unlock(struct conn_table_st *ct);

#endif

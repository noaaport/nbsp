/*
 * Copyright (c) 2006-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ce.h"

/*
 * These functions operate on a conn_element. To give flexibility
 * to the application, we split the initialization routine into four
 * parts. init1 should be called first, and init4 last. init2 and init3
 * can be called anywhere in between. Instead if init2 and init3, the
 * equivalent functions from conn.h to set the appdata and filterdata
 * can be used as well.
 */
int conn_element_init1(struct conn_element_st *ce,
		       int fd,
		       int type, pid_t pid, char *ip, char *name,
		       int write_timeout_ms,
		       int write_timeout_retry,
		       int reconnect_wait_sleep_secs,
		       int reconnect_wait_sleep_retry,
		       int queue_read_timeout_ms){
/*
 * Common initialization for clients and server.
 */
  conn_stats_init(&ce->cs);
  ce->type = type;
  ce->pid = pid;
  ce->ip = ip;		/* must be free()'d */
  ce->name = name;	/* must be free()'d */
  ce->fd = fd;		/* This is a copy of the ct->pfd[] */
  ce->write_timeout_ms = write_timeout_ms; /* timeout writing to clients */
  ce->write_timeout_retry = write_timeout_retry;
  ce->reconnect_wait_sleep_secs = reconnect_wait_sleep_secs;
  ce->reconnect_wait_sleep_retry = reconnect_wait_sleep_retry;
  ce->queue_read_timeout_ms = queue_read_timeout_ms;

  /*
   * These are initialized separately.
   */
  ce->appdata = NULL;
  ce->freedataproc = NULL;
  ce->filterdata = NULL;
  ce->freefilterdataproc = NULL;
  ce->thkillproc = NULL;
  ce->cq = NULL;
  ce->threadinfo = NULL;

  ce->f_exit = 0;

  return(0);
}

int conn_element_init2(struct conn_element_st *ce,
		       void *appdata, free_appdata_proc freedataproc){
  /*
   * This function does the same as conn_table_set_element_appdata().
   */
  ce->appdata = appdata;
  ce->freedataproc = freedataproc;

  return(0);
}

int conn_element_init3(struct conn_element_st *ce, void *filterdata,
		       free_filterdata_proc freefilterdataproc){
  /*
   * This function does the same as conn_table_set_element_filterdata().
   */
  ce->filterdata = filterdata;
  ce->freefilterdataproc = freefilterdataproc;

  return(0);
}

int conn_element_init4(struct conn_element_st *ce,
		      thread_create_proc thcreateproc,
		      thread_kill_proc thkillproc,
		      struct connqueue_param_st *cqparam, int *dberror){
  pthread_t thread_id;
  int dberror_ignore;
  int status = 0;

  ce->threadinfo =
    (struct thread_info_st*)malloc(sizeof(struct thread_info_st));
  if(ce->threadinfo == NULL)
    return(-1);

  (ce->threadinfo)->f_thread_created = 0;
  (ce->threadinfo)->f_thread_finished = 0;

  (ce->threadinfo)->connection_status = 0;
  (ce->threadinfo)->client_fd = ce->fd;

  ce->thkillproc = thkillproc;

  status = pthread_mutex_init(&(ce->threadinfo)->mutex, NULL);
  if(status != 0){
    ce->thkillproc = NULL;
    free(ce->threadinfo);
    ce->threadinfo = NULL;
    return(-1);
  }

  ce->cq = connqueue_open(cqparam, dberror);
  if(ce->cq == NULL){
    (void)pthread_mutex_destroy(&(ce->threadinfo)->mutex);
    ce->thkillproc = NULL;
    free(ce->threadinfo);
    ce->threadinfo = NULL;
    return(-1);
  }

  status = thcreateproc(ce, &thread_id);
  if(status != 0){
    (void)connqueue_close(ce->cq, &dberror_ignore);
    (void)pthread_mutex_destroy(&(ce->threadinfo)->mutex);
    ce->thkillproc = NULL;
    free(ce->threadinfo);
    ce->threadinfo = NULL;
    return(-1);
  }

  (ce->threadinfo)->thread_id = thread_id;
  (ce->threadinfo)->f_thread_created = 1;

  return(0);
}

int conn_element_release(struct conn_element_st *ce, int *dberror){
  /*
   * The ce->fd should not be closed, since it is just a copy of
   * the ct->pfd[], and that will be closed (by conn_table_del_element())
   * after this function is called.
   */
  int status = 0;
  int status1 = 0;

  if(ce->threadinfo != NULL){
   if((ce->threadinfo)->f_thread_created){
     status = ce->thkillproc(ce);
   }
   (void)pthread_mutex_destroy(&(ce->threadinfo)->mutex);
   free(ce->threadinfo);
  }

  if(ce->ip != NULL)
    free(ce->ip);

  if(ce->name != NULL)
    free(ce->name);

  if(ce->appdata != NULL)
    ce->freedataproc(ce->appdata);

  if(ce->filterdata != NULL)
    ce->freefilterdataproc(ce->filterdata);

  if(ce->cq != NULL){
    status1 = connqueue_close(ce->cq, dberror);
    if(status == 0)
      status = status1;
  }
  
  return(status);
}

int conn_element_isclient(struct conn_element_st *ce){

  if((ce->type == CONN_TYPE_CLIENT_LOCAL) ||
     (ce->type == CONN_TYPE_CLIENT_NET)){
    return(1);
  }

  return(0);
}

int conn_element_isnetclient(struct conn_element_st *ce){

  if(ce->type == CONN_TYPE_CLIENT_NET){
    return(1);
  }

  return(0);
}

int conn_element_isnetclient_running(struct conn_element_st *ce){

  int isnetclient;
  int f_created_flag;
  int f_finished_flag;

  isnetclient = conn_element_isnetclient(ce);
  f_created_flag = conn_element_get_created_flag(ce);
  f_finished_flag = conn_element_get_finished_flag(ce);

  if(isnetclient && f_created_flag && (f_finished_flag == 0))
    return(1);

  return(0);
}

int conn_element_isnetclient_stoped(struct conn_element_st *ce){


  int isnetclient;
  int f_created_flag;
  int f_finished_flag;

  isnetclient = conn_element_isnetclient(ce);
  f_created_flag = conn_element_get_created_flag(ce);
  f_finished_flag = conn_element_get_finished_flag(ce);

  if(isnetclient && f_created_flag && f_finished_flag)
    return(1);

  return(0);
}

int conn_element_isserver(struct conn_element_st *ce){

  if((ce->type == CONN_TYPE_SERVER_LOCAL) ||
     (ce->type == CONN_TYPE_SERVER_NET)){
    return(1);
  }

  return(0);
}

int conn_element_get_fd(struct conn_element_st *ce){

  return(ce->fd);
}

void conn_element_set_fd(struct conn_element_st *ce, int fd){

  ce->fd = fd;
}

int conn_element_get_pid(struct conn_element_st *ce){

  return(ce->pid);
}

int conn_element_get_type(struct conn_element_st *ce){

  return(ce->type);
}

char *conn_element_get_ip(struct conn_element_st *ce){

  return(ce->ip);
}

char *conn_element_get_name(struct conn_element_st *ce){

  return(ce->name);
}

char *conn_element_get_nameorip(struct conn_element_st *ce){

  if(ce->name != NULL)
    return(ce->name);

  return(ce->ip);
}

void *conn_element_get_appdata(struct conn_element_st *ce){

  return(ce->appdata);
}

void *conn_element_get_filterdata(struct conn_element_st *ce){
     
  return(ce->filterdata);
}

connqueue_t *conn_element_get_queue(struct conn_element_st *ce){

  return(ce->cq);
}

struct conn_stats_st *conn_element_get_cstats(struct conn_element_st *ce){

  return(&ce->cs);
}

int conn_element_report_cstats(struct conn_element_st *ce,
			       int count, time_t period, char *fname){
  /*
   * This is a utility function for the client threads.
   * If the threads use the same file for logging, they must synchronize
   * the access, or leave it to fopen().
   */
  char *nameorip;
  time_t now;
  FILE *f;
  struct conn_stats_st *cs = &ce->cs;

  if(count > 0){
    /*
     * Log if the number of packets received (since last reset) is
     * greater than the configured packet count.
     */
    if(cs->packets_rtime < (unsigned int)count)
      return(0);

    now = time(NULL);
  }else{
    /*
     * Log if the current time is greater than the last reset time
     * plus the configured period.
     */
    now = time(NULL);
    if(now < (cs->rtime + period))
      return(0);
  }

  nameorip = conn_element_get_name(ce);
  if(nameorip == NULL)
    nameorip = conn_element_get_ip(ce);

  if(fname == NULL){
    f = stdout;
  } else {
    f = fopen(fname, "a");
    if(f == NULL)
      return(-1);
  }

  fprintf(f, "%s" " %" PRIuMAX " %" PRIuMAX " %u %u %.1g"
	  " %" PRIuMAX " %u %u %.1g\n",
	  nameorip, (uintmax_t)now,
	  (uintmax_t)cs->ctime,
	  cs->errors_ctime,
	  cs->packets_ctime,
	  cs->bytes_ctime,
	  (uintmax_t)cs->rtime,
	  cs->errors_rtime,
	  cs->packets_rtime,
	  cs->bytes_rtime);

  if(f != stdout)
    fclose(f);

  conn_stats_reset(&ce->cs);

  return(0);
}

time_t conn_element_get_ctime(struct conn_element_st *ce){

  return(conn_stats_get_ctime(&ce->cs));
}

int conn_element_get_created_flag(struct conn_element_st *ce){

  if(ce->threadinfo == NULL)
    return(0);

  return((ce->threadinfo)->f_thread_created);
}

int conn_element_get_finished_flag(struct conn_element_st *ce){

  int status = 0;
  int finished;

  if(ce->threadinfo == NULL)
    return(0);

  /*
   * Get the flag even in case of error from pthread_mutex_lock.
   */
  status = pthread_mutex_lock(&(ce->threadinfo)->mutex);
  finished = (ce->threadinfo)->f_thread_finished;
  if(status == 0){
    (void)pthread_mutex_unlock(&(ce->threadinfo)->mutex);
  }

  return(finished);
}

int conn_element_set_finished_flag(struct conn_element_st *ce){

  int status = 0;

  assert(ce->threadinfo != NULL);
  if(ce->threadinfo == NULL)
    return(-1);

  /*
   * Set the flag even in case of error.
   */
  status = pthread_mutex_lock(&(ce->threadinfo)->mutex);
  (ce->threadinfo)->f_thread_finished = 1;
  if(status == 0){
    status = pthread_mutex_unlock(&(ce->threadinfo)->mutex);
  }

  return(status);
}

int conn_element_get_connection_status(struct conn_element_st *ce,
					    int *fd){
  /*
   * If the threadinfo is NULL, the connection status
   * is returned as zero (i.e., no errors), since the server thread has
   * not yet been created, and the connection status is set by the
   * server thread to indicate a connection error.
   */
  int status;

  if(ce->threadinfo == NULL)
    return(0);

  if(pthread_mutex_lock(&(ce->threadinfo)->mutex) != 0)
    return(-1);

  status = (ce->threadinfo)->connection_status;
  if((status == 0) && (fd != NULL))
    *fd = (ce->threadinfo)->client_fd;

  if(pthread_mutex_unlock(&(ce->threadinfo)->mutex) != 0)
    return(-1);

  return(status);
}

int conn_element_set_connection_status(struct conn_element_st *ce,
				       int fd){
  /*
   * If fd == -1, the status is set to 1; otherwise it is set to 0.
   * The threadinfo->client_fd is set to the value of fd.
   */

  assert(ce->threadinfo != NULL);
  if(ce->threadinfo == NULL)
    return(-1);

  if(pthread_mutex_lock(&(ce->threadinfo)->mutex) != 0)
    return(-1);

  (ce->threadinfo)->client_fd = fd;
  if(fd == -1)
    (ce->threadinfo)->connection_status = 1;
  else
    (ce->threadinfo)->connection_status = 0;

  if(pthread_mutex_unlock(&(ce->threadinfo)->mutex) != 0)
    return(-1);

  return(0);
}

int conn_element_get_connection_status_un(struct conn_element_st *ce){
  /*
   * This is an unlocked version to read the status variable. This is what
   * the main server uses to loop through the table and find and process
   * clients that have disconnected.
   * (See serverm.c/process_dirty_connections())
   */
  int status;

  if(ce->threadinfo == NULL)
    return(-1);

  status = (ce->threadinfo)->connection_status;

  return(status);
}

void conn_element_set_exit_flag(struct conn_element_st *ce){

  ce->f_exit = 1;
}

int conn_element_get_exit_flag(struct conn_element_st *ce){

  return(ce->f_exit);
}

int conn_element_get_write_timeout_ms(struct conn_element_st *ce){

  return(ce->write_timeout_ms);
}

int conn_element_get_write_timeout_retry(struct conn_element_st *ce){

  return(ce->write_timeout_retry);
}

int conn_element_get_reconnect_wait_sleep_secs(struct conn_element_st *ce){

  return(ce->reconnect_wait_sleep_secs);
}

int conn_element_get_reconnect_wait_sleep_retry(struct conn_element_st *ce){

  return(ce->reconnect_wait_sleep_retry);
}

int conn_element_get_queue_read_timeout_ms(struct conn_element_st *ce){

  return(ce->queue_read_timeout_ms);
}

/*
 * Theese parameters are initialized in init1, but
 * these functions can be used by the application at any time
 * to modify the value (on a per-host basis if desired).
 */
void conn_element_set_write_timeout_ms(struct conn_element_st *ce,
				       int write_timeout_ms){

  ce->write_timeout_ms = write_timeout_ms;
}

void conn_element_set_write_timeout_retry(struct conn_element_st *ce,
					  int write_timeout_retry){

  ce->write_timeout_retry = write_timeout_retry;
}

void conn_element_set_reconnect_wait_sleep_secs(struct conn_element_st *ce,
	                               int reconnect_wait_sleep_secs){

  ce->reconnect_wait_sleep_secs = reconnect_wait_sleep_secs;
}

void conn_element_set_reconnect_wait_sleep_retry(struct conn_element_st *ce,
	                               int reconnect_wait_sleep_retry){

  ce->reconnect_wait_sleep_retry = reconnect_wait_sleep_retry;
}

void conn_element_set_queue_read_timeout_ms(struct conn_element_st *ce,
					    int queue_read_timeout_ms){

  ce->queue_read_timeout_ms = queue_read_timeout_ms;
}

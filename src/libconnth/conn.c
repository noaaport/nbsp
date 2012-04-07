/*
 * Copyright (c) 2002-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "conn.h"
#include "ce.h"

#define INITIAL_SIZE	1
#define GROW_FACTOR	2

static int conn_table_grow(struct conn_table_st *ct);

/*
 * The functions that are not protected by the mutex have their name
 * ending with "_un".
 */
struct conn_table_st *conn_table_create(int maxclients,
					conn_handler_proc connproc,
					hangup_handler_proc hangupproc,
					client_access_proc accessproc){
  struct conn_table_st *ct;
  struct pollfd *pfd;
  struct conn_element_st *ce;
  size_t size;

  ct = malloc(sizeof(struct conn_table_st));
  if(ct == NULL)
    return(NULL);

  size = sizeof(struct pollfd) * INITIAL_SIZE;
  pfd = malloc(size);
  if(pfd == NULL){
    free(ct);
    return(NULL);
  }

  size = sizeof(struct conn_element_st) * INITIAL_SIZE;
  ce = malloc(size);
  if(ce == NULL){
    free(pfd);
    free(ct);
    return(NULL);
  }

  if(pthread_mutex_init(&ct->mutex, NULL) != 0){
    free(ce);
    free(pfd);
    free(ct);
    return(NULL);
  }
  
  ct->pfd = pfd;
  ct->ce = ce;
  ct->n = 0;
  ct->nmax = INITIAL_SIZE;
  ct->nclients = 0;
  ct->nclientsmax = maxclients;
  ct->connproc = connproc;
  ct->hangupproc = hangupproc;
  ct->accessproc = accessproc;
  ct->status = 0;
  ct->dberror = 0;

  return(ct);
}

void conn_table_destroy(struct conn_table_st *ct){

  int i;
  int dberror;

  assert((ct != NULL) && (ct->ce != NULL) && (ct->pfd != NULL));

  for(i = 0; i < ct->n; ++i){
    if((ct->pfd[i].fd != -1) && conn_element_isclient(&ct->ce[i])){
      close(ct->pfd[i].fd);
    }
    (void)conn_element_release(&(ct->ce[i]), &dberror);
  }

  pthread_mutex_destroy(&ct->mutex);
  free(ct->ce);  
  free(ct->pfd);
  free(ct);
}

int conn_table_add_element_un(struct conn_table_st *ct,
			      int fd,
			      int type, pid_t pid, char *ip,char *name,
			      int write_timeout_ms,
			      int write_timeout_retry,
			      int reconnect_wait_sleep_secs,
			      int reconnect_wait_sleep_retry,
			      int queue_read_timeout_ms){
  /*
   * This is the mutex-unlocked version.
   */
  int status = 0;
  int n;

  if(ct->n == ct->nmax){
    status = conn_table_grow(ct);
    if(status != 0)
      return(-1);
  }

  n = ct->n;
  ct->pfd[n].fd = fd;
  ct->pfd[n].events = POLLIN;
  ct->pfd[n].revents = 0;	/* see comment in poll_loop_wait() */

  status = conn_element_init1(&(ct->ce[n]), fd, type, pid, ip, name,
			      write_timeout_ms,
			      write_timeout_retry,
			      reconnect_wait_sleep_secs,
			      reconnect_wait_sleep_retry,
			      queue_read_timeout_ms);
  ct->status = status;
  ct->dberror = 0;

  if(status == 0){
    ++ct->n;
    if(type == CONN_TYPE_CLIENT_NET)
      ++ct->nclients;
  }

  return(status);
}

int conn_table_del_element_un(struct conn_table_st *ct, int i){
  /*
   * This is the mutex-unlocked version.
   */
  int status = 0;
  int status1;
  int dberror = 0;
  size_t size;
  int isclient;
  int isnetclient;

  assert((i >= 0) && (i < ct->n));

  /* save it for use below */
  isclient = conn_element_isclient(&ct->ce[i]);
  isnetclient = conn_element_isnetclient(&ct->ce[i]);

  /*
   * Stop the (client) thread first.
   */
  status = conn_element_release(&ct->ce[i], &dberror);
  ct->status = status;
  ct->dberror = dberror;

  if((ct->pfd[i].fd != -1) && isclient){
    status1 = close(ct->pfd[i].fd);
    if(status == 0)
      status = status1;
  }

  /*
   * From the beginning of the array, up to this ith element, there are
   * (i + 1) elements. So there are n - (i + 1) elements above this
   * ith element, which we move by one index.
   */
  size = (ct->n - (i + 1)) * sizeof(struct pollfd);
  memmove(&ct->pfd[i], &ct->pfd[i + 1], size);
  size = (ct->n - (i + 1)) * sizeof(struct conn_element_st);
  memmove(&ct->ce[i], &ct->ce[i + 1], size);
  --ct->n;

  /*
   * If it was a network client, decrease the nclients counter.
   */
  if(isnetclient)
    --ct->nclients;

  return(status);
}

static int conn_table_grow(struct conn_table_st *ct){

  int status = 0;
  struct pollfd *pfd;
  struct conn_element_st *ce;
  int new_nsize;
  int old_nsize;
  size_t size;

  old_nsize = ct->nmax;
  new_nsize = ct->nmax * GROW_FACTOR;

  size = sizeof(struct pollfd) * new_nsize;
  pfd = realloc(ct->pfd, size);
  if(pfd == NULL){
    return(-1);
  }

  size = sizeof(struct conn_element_st) * new_nsize;
  ce = realloc(ct->ce, size);
  if(ce == NULL){
    size = sizeof(struct pollfd) * old_nsize;
    ct->pfd = realloc(pfd, size);
    return(-1);
  }

  ct->pfd = pfd;
  ct->ce = ce;
  ct->nmax = new_nsize;

  return(status);
}

int conn_table_get_element_fd(struct conn_table_st *ct, int i){

  assert((i >= 0) && (i < ct->n));

  return(ct->pfd[i].fd);
}

int conn_table_get_element_type(struct conn_table_st *ct, int i){

  assert((i >= 0) && (i < ct->n));

  return(ct->ce[i].type);
}

int conn_table_get_element_pid(struct conn_table_st *ct, int i){

  assert((i >= 0) && (i < ct->n));

  return(ct->ce[i].pid);
}

char *conn_table_get_element_ip(struct conn_table_st *ct, int i){

  assert((i >= 0) && (i < ct->n));

  return(ct->ce[i].ip);
}

char *conn_table_get_element_name(struct conn_table_st *ct, int i){

  assert((i >= 0) && (i < ct->n));

  return(ct->ce[i].name);
}

char *conn_table_get_element_nameorip(struct conn_table_st *ct, int i){

  assert((i >= 0) && (i < ct->n));

  return(conn_element_get_nameorip(&ct->ce[i]));
}

connqueue_t *conn_table_get_element_queue(struct conn_table_st *ct, int i){

  assert((i >= 0) && (i < ct->n));

  return(ct->ce[i].cq);
}

int conn_table_get_element_fthread_created(struct conn_table_st *ct, int i){

  assert((i >= 0) && (i < ct->n));

  return(conn_element_get_created_flag(&ct->ce[i]));
}

int conn_table_get_element_fthread_finished(struct conn_table_st *ct, int i){

  assert((i >= 0) && (i < ct->n));

  return(conn_element_get_finished_flag(&ct->ce[i]));
}

void *conn_table_get_element_appdata(struct conn_table_st *ct, int i){

  assert((i >= 0) && (i < ct->n));

  return(ct->ce[i].appdata);
}

void conn_table_set_element_appdata(struct conn_table_st *ct, int i,
				   void *data, free_appdata_proc freedataproc){

  assert((i >= 0) && (i < ct->n));

  ct->ce[i].appdata = data;
  ct->ce[i].freedataproc = freedataproc;
}

void *conn_table_get_element_filterdata(struct conn_table_st *ct, int i){

  assert((i >= 0) && (i < ct->n));

  return(ct->ce[i].filterdata);
}

void conn_table_set_element_filterdata(struct conn_table_st *ct,
				      int i, void *filterdata,
				      free_filterdata_proc freefilterdataproc){
  assert((i >= 0) && (i < ct->n));

  ct->ce[i].filterdata = filterdata;
  ct->ce[i].freefilterdataproc = freefilterdataproc;
}

int conn_table_get_numentries(struct conn_table_st *ct){
  /*
   * Total number of occupied entries.
   */
  return(ct->n);
}

int conn_table_get_netclients(struct conn_table_st *ct){
  /*
   * Number of occupied entries that are network clients.
   */

  return(ct->nclients);
}

int conn_table_get_netclientsmax(struct conn_table_st *ct){
  /*
   * Maximum number number network clients allowed, as specified
   * in the initialization function.
   */
  return(ct->nclientsmax);
}

int conn_table_element_isnetclient(struct conn_table_st *ct, int i){

  return(conn_element_isnetclient(&ct->ce[i]));
}

int conn_table_element_isnetclient_running(struct conn_table_st *ct, int i){

  return(conn_element_isnetclient_running(&ct->ce[i]));
}

int conn_table_element_isnetclient_stoped(struct conn_table_st *ct, int i){

  return(conn_element_isnetclient_stoped(&ct->ce[i]));
}

time_t conn_table_get_element_ctime(struct conn_table_st *ct, int i){

  return(conn_element_get_ctime(&ct->ce[i]));
}

int conn_table_find_element_byip(struct conn_table_st *ct, char *ip){
  /*
   * Returns -1 if no element matches the ip.
   */
  int i;
  char *ceip;

  for(i = 0; i < ct->n; ++i){
    ceip = conn_table_get_element_ip(ct, i);
    if(ceip == NULL)
      continue;

    if(strcmp(ceip, ip) == 0)
      return(i);
  }
  
  return(-1);
}

/*
 * The mutex-locked. These are the only functions that lock the mutex
 * of the connection table. See the note below.
 */
int conn_table_add_element(struct conn_table_st *ct,
			   int fd,
			   int type, pid_t pid, char *ip, char *name,
			   int write_timeout_ms,
			   int write_timeout_retry,
			   int reconnect_wait_sleep_secs,
			   int reconnect_wait_sleep_retry,
			   int queue_read_timeout_ms){
  int status = 0;

  if(pthread_mutex_lock(&ct->mutex) != 0)
    return(-1);

  status = conn_table_add_element_un(ct, fd, type, pid, ip, name,
				     write_timeout_ms,
				     write_timeout_retry,
				     reconnect_wait_sleep_secs,
				     reconnect_wait_sleep_retry,
				     queue_read_timeout_ms);
  (void)pthread_mutex_unlock(&ct->mutex);
  
  return(status);
}

int conn_table_del_element(struct conn_table_st *ct, int i){

  int status = 0;

  if(pthread_mutex_lock(&ct->mutex) != 0)
    return(-1);

  status = conn_table_del_element_un(ct, i);
  (void)pthread_mutex_unlock(&ct->mutex);

  return(status);
}

/*
 * These are two utility functions. The only functions that lock
 * the conn table mutex are conn_table_add_element() and
 * conn_table_del_element(). When all the functions of the library are
 * called by only one thread, there is no need to lock. But if another
 * thread (in addition to the server thread) has access to the conn table,
 * it must lock the table to prevent the server from adding or deleting
 * elements while the other thread executed. Neither nbsp nor npemwin
 * do this. In both, only the server thread accesses the conn table.
 * (In npemwin the server thread is the main thread.)
 */
int conn_table_mutex_lock(struct conn_table_st *ct){

  if(pthread_mutex_lock(&ct->mutex) != 0)
    return(-1);

  return(0);
}

int conn_table_mutex_unlock(struct conn_table_st *ct){

  if(pthread_mutex_unlock(&ct->mutex) != 0)
    return(-1);

  return(0);
}

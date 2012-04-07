/*
 * This is the main server. It listens for new connections and spawns
 * a new thread for each client.
 */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <netdb.h>
#include "../libconn.h"
#include "common.h"
#include "dbenv.h"
#include "server.h"	/* common for serverm and serverc */

/*
 * static variables
 */
static struct conn_table_st *gct = NULL;
static int gserverfd = -1;
static int gf_quit = 0;
struct connqueue_param_st gcqparam = {NULL, 0, 0, 0};

/*
 * Local functions
 */
static int init_conn_table(void);
static int free_conn_table(void);
static void set_quit_flag(void);
static void errdb(char *s, int dberror);
static int loop(void);
static void write_client_queues(char *data, int data_size);
static void spawn_client_threads(void);

/*
 * Callbacks passed to conn_table_create():
 *
 * conn_handler_proc
 * hangup_handler_proc
 * client_access_proc
 */
static int handle_client_input(struct conn_table_st *ct, int i);
static void handle_client_hangup(struct conn_table_st *ct, int i, int cond);
static int handle_client_access(int client_fd, char *client_ip);
static int client_thread_create(struct conn_element_st *ce, pthread_t *t_id);
static int client_thread_kill(struct conn_element_st *ce);

static int handle_client_input(struct conn_table_st *ct, int i){
  /*
   * The convetion is that function returns 0 or a positive integer
   * in case of an error.
   */
  int status = 0;
  int n;
  char buf[BUFSIZ];
  int fd;
  pid_t pid;
  char *addr;

  fd = conn_table_get_element_fd(ct, i);
  pid = conn_table_get_element_pid(ct, i);
  if(pid == -1){
    addr = conn_table_get_element_ip(ct, i);
    fprintf(stdout, "Connection from %s\n", addr);
  }else
    fprintf(stdout, "Connection from %d\n", pid);

  if((n = read(fd, buf, BUFSIZ - 1)) > 0){
    buf[n] = '\0';
    fprintf(stdout, "%s\n", buf);
    strncpy(buf, "server: Received\n", BUFSIZ - 1);
    buf[BUFSIZ - 1] = '\0';
    write(fd, buf, strlen(buf));
  }else{
    perror("read");
    status = i;
  }

  return(status);
}

static void handle_client_hangup(struct conn_table_st *ct, int i, int cond){
  /*
   * This function is called by poll_client_hangup() in poll.c
   * to notify the application when it has detected that a client has closed,
   * and it is about to delete the corresponding element from the table.
   * It is called if the descriptor has (POLLHUP | POLLERR | POLLNVAL) set;
   * or POLLIN in a condition that a read will return 0 or -1. 
   */
  char *ip;

  ip = conn_table_get_element_ip(ct, i);
  if(cond > 0)
    warnx("Client %s disconnected", ip);
  else
    warn("Client %s:", ip);
}

static int handle_client_access(int client_fd, char *client_ip){

  int allow;
  char *cname;

  allow = 1;

  cname = get_peer_name(client_fd);
  fprintf(stdout, "Connection from %s\n", client_ip);
  fprintf(stdout, "Connection from %s\n", cname);
  free(cname);

  return(allow);
}

static void spawn_client_threads(void){
  /*
   * Loop through the conn table and spawn the client thread for
   * each element that has been spawned yet.
   */
  int i;
  int n;
  int f_thread_created;
  int status;
  int dberror;

  n = conn_table_get_numentries(gct);
  for(i = 0; i < n; ++i){
    f_thread_created = conn_table_get_element_fthread_created(gct, i);
    if(conn_element_isclient(&gct->ce[i]) && (f_thread_created == 0)){
      status = conn_element_init4(&gct->ce[i],
				  client_thread_create,
				  client_thread_kill,
				  &gcqparam,
				  &dberror);
      if(status != 0)
	errdb("Cannot spawn client thread.", dberror);
      else
	fprintf(stdout, "Spawned client thread %d.\n", i);
    }
  }
}

static void join_finished_threads(void){
  /*
   * Loop through the conn table and join any client thread for
   * which the finished flag has been set.
   */
  int i;
  int n;
  int f_thread_finished;
  int f_thread_created;

  n = conn_table_get_numentries(gct);
  for(i = 0; i < n; ++i){
    f_thread_created = conn_table_get_element_fthread_created(gct, i);
    if(f_thread_created == 0)
      continue;

    f_thread_finished = conn_table_get_element_fthread_finished(gct, i);
    if(f_thread_finished == 1){
      fprintf(stdout, "A thread finished.\n");
      poll_kill_client_connection(gct, i);
      n = conn_table_get_numentries(gct);
    }
  }
}

static int client_thread_create(struct conn_element_st *ce, pthread_t *t_id){
  /*
   * This function ends being called by conn_element_init(), which in turn
   * is called by conn_table_add_element(). At that time the ce table
   * is locked, but subsequently the table changes (when other elements
   * are added or deleted). This function must make a copy (used only
   * as read-only) of its ce and work with that throught the thread's lifetime.
   * The thread's main routine must free() that copy when it finishes.
   */
  int status = 0;
  pthread_attr_t attr;
  void *arg;
  struct conn_element_st *ce_copy; 

  ce_copy = malloc(sizeof(struct conn_element_st));
  if(ce_copy == NULL)
    err(1, "Could not create client thread.");

  memcpy(ce_copy, ce, sizeof(struct conn_element_st));
  arg = (void*)ce_copy;
  
  status = pthread_attr_init(&attr);
  if(status == 0)
    status = pthread_create(t_id, &attr, client_thread_main, arg);

  if(status != 0){
    warn("Cannot create client thread.");
  }else{
    fprintf(stderr, "%s\n", "Spawned client thread.");
  }

  return(status);
}

static int client_thread_kill(struct conn_element_st *ce){

  int status = 0;
  void *pthread_status;

  /*
   * If the thread loop function calls get_quit_flag() then this
   * is not strictly necessary, but it can be faster.
   */
  status = pthread_cancel((ce->threadinfo)->thread_id);
  if((status != 0) && (status != ESRCH))
    warnx("Error %d canceling client.", status);

  status = pthread_join((ce->threadinfo)->thread_id, &pthread_status);
  if(status != 0)
    warnx("Error %d joining client thread.", status);
  else if(pthread_status == PTHREAD_CANCELED)
    fprintf(stderr, "%s\n", "Canceled thread.");
  else if(pthread_status == NULL)
    fprintf(stderr, "%s\n", "Finished thread.");

  return(0);
}

static int init_conn_table(void){

  int status;
  int maxclients = -1;
  char *ip = NULL;
  char *name = NULL;
  
  gcqparam.dbenv = dbenv_open();
  if(gcqparam.dbenv == NULL){
    warn("Cannot create dbenv.");
    return(-1);
  }

  gcqparam.reclen = DBQ_RECLEN;
  gcqparam.softlimit = 0;
  gcqparam.hardlimit = 0;

  gct = conn_table_create(maxclients,
			  handle_client_input,
			  handle_client_hangup,
			  handle_client_access);
  if(gct == NULL){
    warn("Cannot create table.");
    return(-1);
  }
  status = conn_table_add_element(gct, gserverfd, CONN_TYPE_SERVER_NET,
                                  0, ip, name);
  if(status != 0){
    err(1, "Could not add element.");
    return(-1);
  }

  return(0);
}
  
static int free_conn_table(void){

  conn_table_destroy(gct);	/* This kills the threads. */
  if(gcqparam.dbenv != NULL)
    dbenv_close(gcqparam.dbenv);

  return(0);
}

int get_quit_flag(void){
  /*
   * No need to lock it since there is no need for strict synchronization.
   */
  int flag;
  
  flag = gf_quit;
  
  return(flag);
}

static void set_quit_flag(void){

  gf_quit = 1;
}

static void errdb(char *s, int dberror){

  if(dberror >= 0){
    /*
     * dberror > 0 => a system error within the db library
     * dberror = 0 => a system error outside the library.
     */
    warn(s);
  }else if(dberror < 0){
    warnx("%s %s", s, db_strerror(dberror));
  }
}

int main(void){

  int status;
  int gai_code;

  gserverfd = tcp_server_open_conn(NULL, NBSP_PORT, 5, NULL, &gai_code);
  if(gserverfd == -1){
    if(gai_code != 0)
      errx(1, "tcp_server. %s", gai_strerror(gai_code));
    else
      err(1, "server_open_nconn()");
  }

  status = init_conn_table();
  
  while((status == 0) && (get_quit_flag() == 0)){
    status = loop();
  }

  free_conn_table();
  close(gserverfd);

  return(0);
}

static int loop(void){
  /*
   * Read a string from stdin, and then send it ten times to
   * the client queues.
   */
  int status;
  char buf[DBQ_RECLEN];
  int i;

  /*
   * Process new connections, hangups, etc.
   */
  status = poll_loop(gct);
  if(status != 0)
    fprintf(stdout, "Status from poll loop: %d\n", status);

  memset(buf, '\0', DBQ_RECLEN);
  while(fgets(buf, DBQ_RECLEN, stdin) != NULL){
    /*
     * Process new connections, hangups, etc.
     */
    status = poll_loop_nowait_nonblock(gct);
    if(status != 0)
      fprintf(stdout, "Status from poll loop: %d\n", status);

    if(strncmp(buf, "quit", 4) == 0){
      set_quit_flag();
      break;
    }
    for(i = 1; i <= 4; ++i){
      write_client_queues(buf, DBQ_RECLEN);
    }

    join_finished_threads();
    spawn_client_threads();
  }

  return(0);
}

static void write_client_queues(char *data, int data_size){
  /*
   * Send to the queues of all the running client threads.
   */
  int i;
  int dberror;
  int status = 0;
  int count = 0;
  int f_thread_created;
  int type;

  for(i = 0; i < gct->n; ++i){
    f_thread_created = conn_table_get_element_fthread_created(gct, i);
    type = conn_table_get_element_type(gct, i);
    if(type == CONN_TYPE_CLIENT_NET){
      if(f_thread_created){
	++count;
	status = connqueue_snd(gct->ce[i].cq, data, data_size, &dberror);
	if(status != 0)
	  errdb("Error writing to client queue.", dberror);
      } else
	warnx("Client %d not ready.", i);
    }
  }
  fprintf(stdout, "Sent to %d connections\n", count);
}

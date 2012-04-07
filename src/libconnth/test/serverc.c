/*
 * The client thread functions. The main function is what the
 * main server inserts in pthread_create.
 */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include "../libconn.h"
#include "server.h"	/* common for serverm and serverc */

static void client_thread_loop(struct conn_element_st *ce);
static void client_thread_cleanup(void*);
static void periodic(struct conn_element_st *ce);

void *client_thread_main(void *arg){
  /*
   * The *arg is a private copy of the ce. This function must
   * free(ce) it when it is finished, but it must try to
   * delete anything else from the ce.
   */
  struct conn_element_st *ce = (struct conn_element_st*)arg;

  pthread_cleanup_push(client_thread_cleanup, arg);

  while((get_quit_flag() == 0) && (conn_element_get_exit_flag(ce) == 0)){
    periodic(ce);
    client_thread_loop(ce);
  }

  pthread_cleanup_pop(1);

  return(NULL);
}

static void client_thread_loop(struct conn_element_st *ce){
  /*
   * The only job of this function is to read from the ce->cq queuee, and
   * write to the ce->fd client file descriptor. The *data returned
   * points to a resusable internal storage in the ce->cq, and
   * should not be free()'d explicitly. It is released when the
   * main thread deletes the client from the table.
   */
  int status;
  int dberror;
  int timeout_ms = 10000;
  void *data = NULL;
  uint32_t data_size;

  status = connqueue_rcv(ce->cq, &data, &data_size, timeout_ms, &dberror);
  if(status == -1){
    if(dberror == 0)
      warn("Error from connqueue_rcv().");
    else
      warnx("DB error %d from connqueue_rcv().", dberror);
  } else if(status == 1)
    warnx("No data in queue.");

  if(status != 0)
    return;

  fprintf(stdout, "Thread read from ce->cq: %d: %s",
	  data_size, (char*)data);

  /*
   * Send to client.
   */
  if(write(ce->fd, data, data_size) == -1){
    conn_stats_update_errors(&ce->cs);
    warn("Error writing to client.");
    return;
  }
  
  /* 
   * This fakes exiting from an unrrecoverable (e.g., write) error.
   *
   *   conn_element_set_exit_flag(ce);
   */

  conn_stats_update_packets(&ce->cs, data_size);
}

static void client_thread_cleanup(void *arg){
  /*
   * See note in client_thread_main().
   */
  struct conn_element_st *ce = (struct conn_element_st*)arg;

  /*
   * The finished  flag is set here so that if the thread is exiting
   * as a result of having called pthread_exit (e.g., if it encountered
   * an irrecoverable error) the main thread can check and remove
   * the connection element from the table.
   */

  conn_element_set_finished_flag(ce);
  free(ce);
}

static void periodic(struct conn_element_st *ce){

  conn_element_report_cstats(ce, 2, NULL);
  /*
   *   conn_element_report_cstats(ce, 2, "server.log");
   */
}

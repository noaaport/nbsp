/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

/*
 * This file has the client thread functions. The main function here
 * is what the main server inserts in pthread_create when it calls
 * client_thread_create(). The main server thread is in serverm.c.
 */
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "libconnth/libconn.h"
#include "readn.h"
#include "globals.h"
#include "err.h"
#include "signal.h"
#include "appdata.h"
#include "emwin.h"
#include "packfpu.h"
#include "nbs1.h"
#include "server_priv.h"

static void *client_thread_main(void *arg);
static void loop(struct conn_element_st *ce);
static void cleanup(void*);
static void periodic(struct conn_element_st *ce);

static int send_emwin_client(struct conn_element_st *ce,
			     struct emwin_queue_info_st *eqinfo,
			     uint32_t eqinfo_size);
static int send_nbs1_client(struct conn_element_st *ce,
			    void *packet, uint32_t p_size);
static int send_nbs2_client(struct conn_element_st *ce,
			    void *packet, uint32_t p_size);

static int emwin_retransmit_packet(struct conn_element_st *ce,
				   struct emwin_packet_st *ep);
static int nbs_retransmit_packet(struct conn_element_st *ce,
				 struct nbs1_packet_st *nbs);

static int send_client(struct conn_element_st *ce,
		       void *data, uint32_t data_size);
static int send_client1(struct conn_element_st *ce,
			void *data, uint32_t data_size);
static int wait_client_reconnection(struct conn_element_st *ce);

int client_thread_create(struct conn_element_st *ce, pthread_t *t_id){
  /*
   * This function is what is passed to conn_element_init4().
   * At that time the ce table is locked, but subsequently
   * the table changes (when other elements are added or deleted).
   * This function must make a copy (used only as read-only)
   * of the ce and the client thread must work with that throught its lifetime.
   * The client thread's main routine client_thread_main() must free()
   * that copy when it finishes. The function client_thread_main()
   * is declared in server_priv.h and defined in serverc.c.
   */
  int status = 0;
  pthread_attr_t attr;
  void *arg;
  struct conn_element_st *ce_copy;
  char *nameorip;

  nameorip = conn_element_get_nameorip(ce);

  ce_copy = malloc(sizeof(struct conn_element_st));
  if(ce_copy == NULL){
    log_err2("Could not create client thread for", nameorip);
    return(-1);
  }

  memcpy(ce_copy, ce, sizeof(struct conn_element_st));
  arg = (void*)ce_copy;
  
  status = pthread_attr_init(&attr);
  if(status == 0)
    status = pthread_create(t_id, &attr, client_thread_main, arg);

  if(status != 0){
    free(ce_copy);
    log_errx("Error %d creating client thread for %s.", status, nameorip);
  } else {
    log_info("Created client thread for %s.", nameorip);
  }

  return(status);
}

int client_thread_kill(struct conn_element_st *ce){
  /*
   * This function is what is passed to conn_element_init4().
   */
  int status = 0;
  void *pthread_status;
  char *nameorip;

  nameorip = conn_element_get_nameorip(ce);

  /*
   * If the thread loop function calls get_quit_flag() then it is not
   * necessary to call pthread_cancel for the final termination. But in cases
   * in which the server wants to terminate the thread early due to
   * errors this is the only way to notify the client thread.
   */
  status = pthread_cancel((ce->threadinfo)->thread_id);
  if((status != 0) && (status != ESRCH))
    log_errx("Error %d canceling client thread %s.", status, nameorip);

  status = pthread_join((ce->threadinfo)->thread_id, &pthread_status);
  if(status != 0)
    log_errx("Error %d joining client thread %s.", status, nameorip);
  else if(pthread_status == PTHREAD_CANCELED)
    log_info("Canceled client thread %s.", nameorip);
  else if(pthread_status == NULL)
    log_info("Finished client thread %s.", nameorip);

  return(0);
}

static void *client_thread_main(void *arg){
  /*
   * The *arg is a private copy of the ce. This function must
   * free(ce) it when it is finished, but it must _not_ try to
   * delete anything else from the ce.
   * We used to set the cancellation state to DISABLED and then renabled
   * after the loop in the loop, to avoid cancelling the thread while
   * it holds a mutex locked. But this not appropriate if we let the
   * g.client_queue_read_timeout_ms be a configurable parameter: the user
   * can set this very high, and the thread would not be canceled until
   * that timer expires. [The alternative would be to write a wrapper
   * over connqueue_rcv() that disbales the cancellation and calls
   * connqueue_rcv() with a short (i.e., 1 second) wait time
   * and retries any number of times.]
   *
   * See cleanup().
   */
  /* int cancel_state; */
  /* int status = 0; */
  struct conn_element_st *ce = (struct conn_element_st*)arg;

  pthread_cleanup_push(cleanup, arg);

  while((get_quit_flag() == 0) && (conn_element_get_exit_flag(ce) == 0)){
    pthread_testcancel();

    /*
     * status = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cancel_state);
     */

    periodic(ce);
    loop(ce);

    /*
     * status = pthread_setcancelstate(cancel_state, &cancel_state);
     */
  }

  pthread_cleanup_pop(1);

  return(NULL);
}

static void loop(struct conn_element_st *ce){
  /*
   * The only job of this function is to read from the ce->cq, and
   * write to the ce->fd client file descriptor. The *data returned
   * points to a reusable internal storage in the ce->cq, and
   * should not be free()'d explicitly. It is released when the
   * main thread deletes the client from the table.
   */
  int status;
  int dberror;
  void *data = NULL;
  uint32_t data_size;
  int timeout_ms = conn_element_get_queue_read_timeout_ms(ce);

  /*
   * The nbs1 and nbs2 threads receive the packetinfo->packet while
   * the emwin threads receive a "struct emwin_queue_info_st".
   */
  status = connqueue_rcv(ce->cq, &data, &data_size, timeout_ms, &dberror);
  if(status == -1)
    log_err2_db("Error reading from queue for %s.",
		conn_element_get_nameorip(ce), dberror);
  else if(status == 1)
    log_errx("No data in queue for %s.", conn_element_get_nameorip(ce));

  if(status != 0)
    return;

  status = send_client(ce, data, data_size);

  if(status != 0){
    conn_stats_update_errors(&ce->cs);
    conn_element_set_exit_flag(ce);
  }
}

static void cleanup(void *arg){
  /*
   * See note in client_thread_main().
   *
   * We should call connqueue_rcv_cleanup() here to
   * unlock the libqdb mutex since it is posible that the thread was
   * canceled while waiting on the condition variable in connqueue_rcv().
   * If we disable the cancellation while calling connqueue_rcv() there
   * is no need to do that, but since we are not disabling the cancelation
   * (as we used to) we must call connqueue_rcv_cleanup().
   */
  int status;
  int dberror;
  struct conn_element_st *ce = (struct conn_element_st*)arg;

  status = connqueue_rcv_cleanup(ce->cq, &dberror);
  if(status != 0)
    log_err_db("Error from connqueue_rcv_cleanup()", dberror);

  /*
   * The thread_finished flag is set here so that if the thread is exiting
   * by itself (e.g., if it encountered an unrrecoverable error from write())
   * the main thread can check and remove the connection element
   * from the table.
   */

  status = conn_element_set_finished_flag(ce);
  if(status != 0){
    /*
     * This really should not happen; if it does it is a bug.
     */
    log_err("Error from conn_element_set_finished_flag().");
  }

  free(ce);
}

static void periodic(struct conn_element_st *ce){
  /*
   * When the period defined in the variable
   * 
   *	g.serverthreads_logperiod_secs
   *
   * has passed, this function will write the stats summary since
   * the last time that it was called.
   *
   * Also check the status flag of the queue. The flags are defined
   * in libqdb/qdb.h, and they are set by the server via the libqdb snd()
   * function. If the hard or soft limit flags are set, we
   * let the thread continue to run to see if the situation normalizes.
   * If the error flag is set, then exit the thread to resync.
   */
  int status = 0;
  char *nameorip;

  if(connqueue_test_dberror_flag(ce->cq) != 0){
      nameorip = conn_element_get_nameorip(ce);
      log_errx("Server set qdb_status flag for %s. Finishing client thread.",
	       nameorip);
    conn_element_set_exit_flag(ce);
    return;
  }

  /*
   * In nbsp the default criterion for loging is the period, and not on the
   * packet count, but that is runtime configurable.
   */
  status = conn_element_report_cstats(ce,
				      g.serverthreads_logperiod_packets,
				      g.serverthreads_logperiod_secs,
				      g.serverthreadsfile);
  if(status != 0)
    log_err_write(g.serverthreadsfile);
}

static int send_emwin_client(struct conn_element_st *ce,
			     struct emwin_queue_info_st *eqinfo,
                             uint32_t eqinfo_size __attribute__((unused))){
  int status = 0;
  struct emwin_packet_st ep;
  int i;

  /*
   * assert(eqinfo_size == emwin_queue_info_get_size());
   */

  status = init_emwin_packet_st(&ep, eqinfo->fpathout, eqinfo->emwinfname);
  if(status == -1)
    log_err2("Could not initialize emwin packet", eqinfo->fpathout); 
  else if(status != 0)
    log_errx("Error from init_emwin_packet() for %s.", eqinfo->fpathout); 

  if(status != 0)
    return(status);

  for(i = 1; i <= ep.parts_total; ++i){
    status = build_emwin_packet(&ep);
    if(status == -1)
      log_err2("Could not build emwin packet", ep.fname); 
    else if(status != 0)
      log_errx("Header error building emwin packet %s.", ep.fname); 

    if(status == 0)
      status = emwin_retransmit_packet(ce, &ep);

    if(status != 0)
      break;
  }

  free_emwin_packet_st(&ep);

  return(status);
}

static int emwin_retransmit_packet(struct conn_element_st *ce,
				   struct emwin_packet_st *ep){
  int fd;
  char *nameorip;
  int timeout_ms;
  int retry;
  int status = 0;

  fd = conn_element_get_fd(ce);
  nameorip = conn_element_get_nameorip(ce);
  timeout_ms = conn_element_get_write_timeout_ms(ce);
  retry = conn_element_get_write_timeout_retry(ce);

  status = send_emwin_packet(fd, ep, timeout_ms, retry);
  if(status == -1)
    log_err2("Cannot transmit to client", nameorip);
  else if(status == 1)
    log_errx("Cannot transmit to client %s. Timed out %d ms.",
	     nameorip, timeout_ms);

  if(status != 0){
    /*
     * Set the connection_status flag to make the main thread
     * (process_dirty_connections() in serverm.c) to
     * close the connection and let the the client to try to restore
     * it since there is no other way to resynchronize the server and client.
     * If there is an error (mutex) trying to raise the flag, then
     * we try to exit the client thread entirely.
     */
    conn_element_set_fd(ce, -1);
    if(conn_element_set_connection_status(ce, -1) != 0)
      conn_element_set_exit_flag(ce);
  } else 
    conn_stats_update_packets(&ce->cs, ep->packet_size);

  if((status == 0) && (g.f_debug != 0))
    log_msg(LOG_DEBUG, "Transmitted part %d of %d to client %s",
	    ep->part_number, ep->parts_total, nameorip);

  return(status);
}

static int send_nbs1_client(struct conn_element_st *ce,
			    void *packet, uint32_t packet_size){
  /*
   * This function receives the packetinfo->packet and
   * packetinfo->packet_size portion of the packetinfo, from
   * which the packetinfo can be reconstructed by calling
   * nbsfp_unpack_fpath(&gpacketinfo);
   */
  int status = 0;
  struct nbs1_packet_st nbs;
  struct packet_info_st packetinfo;
  int i;

  packetinfo.packet_size = (size_t)packet_size;
  packetinfo.packet = packet;

  status = nbsfp_unpack_fpath(&packetinfo);
  if(status != 0){
    log_errx("Corrupted data from queue.");
    return(status);
  }

  /*
   * log_info("XXX send_nbs1_client(): %s[%u]",
   *   packetinfo.fpath, packetinfo.seq_number);
   */

  status = init_nbs_packet_st(&nbs, packetinfo.fpath,
			      packetinfo.seq_number, 
			      packetinfo.psh_product_type,
			      packetinfo.psh_product_category, 
			      packetinfo.psh_product_code,
			      packetinfo.np_channel_index,
			      packetinfo.fname);
  if(status == -1){
    log_err2("Could not initialize nbs packet", packetinfo.fpath); 
    return(status);
  }else if(status == -2){
    log_errx("Could not initialize nbs packet. %s not found in mspoolbdb.",
	     packetinfo.fpath);
    return(status);
  }else if(status != 0){
    /*
     * This is a bug in which init_nbs_packet was called with an invalid
     * value for the spooltype (which should have been checked in conf.c.in).
     */
    log_errx("BUG: init_nbs_packet() called with invalid spooltype");
    return(status);
  }

  for(i = 1; i <= nbs.num_blocks; ++i){
    status = build_nbs_packet(&nbs);
    if(status == -1)
      log_err2u("Could not build nbs packet", packetinfo.seq_number);
    else if(status != 0){
      /*
       * This would be a bug from build_nbs_packet.
       */
      log_errx("BUG: build_nbs_packet() called with all files closed.");
    }
 
    if(status == 0)
      status = nbs_retransmit_packet(ce, &nbs);

    if(status != 0)
      break;
  }

  free_nbs_packet_st(&nbs);

  return(status);
}

static int nbs_retransmit_packet(struct conn_element_st *ce,
				 struct nbs1_packet_st *nbs){
  int fd;
  char *nameorip;
  int timeout_ms;
  int retry;
  int status = 0;

  fd = conn_element_get_fd(ce);
  nameorip = conn_element_get_nameorip(ce);
  timeout_ms = conn_element_get_write_timeout_ms(ce);
  retry = conn_element_get_write_timeout_retry(ce);

  status = send_nbs_packet(fd, nbs, timeout_ms, retry);
  if(status == -1){
    log_err2("Cannot transmit to client", nameorip);
  } else if(status > 0) {
    log_errx("Cannot transmit to client %s. Timed out %d ms.",
	     nameorip, timeout_ms);
  }

  if(status != 0){
    /*
     * Set the connection_status flag to make the main thread
     * (process_dirty_connections() in serverm.c) to
     * close the connection and let the the client to try to restore
     * it since there is no other way to resynchronize the server and client.
     * If there is an error (mutex) trying to raise the flag, then
     * we try to exit the client thread entirely.
     */
    conn_element_set_fd(ce, -1);
    if(conn_element_set_connection_status(ce, -1) != 0)
      conn_element_set_exit_flag(ce);
  } else
    conn_stats_update_packets(&ce->cs, nbs->packet_size);

  if((status == 0) && (g.f_debug != 0))
     log_msg(LOG_DEBUG, "Transmit OK to client %s", nameorip);

  return(status);
}

static int send_nbs2_client(struct conn_element_st *ce,
			    void *packet, uint32_t packet_size){
  int status = 0;
  int fd;
  char *nameorip;
  int timeout_ms;
  int retry;
  ssize_t n = 0;

  fd = conn_element_get_fd(ce);
  nameorip = conn_element_get_nameorip(ce);
  timeout_ms = conn_element_get_write_timeout_ms(ce);
  retry = conn_element_get_write_timeout_retry(ce);

  n = writem(fd, packet, (size_t)packet_size, (unsigned int)timeout_ms, retry);

  if(n == -1){
    status = -1;
    log_err2("Cannot transmit to client", nameorip);
  } else if((size_t)n != packet_size) {
    /*
     * timed out (including partial read)
     */
    status = 1;
    log_errx("Cannot transmit to client %s. Timed out %d ms.",
	     nameorip, timeout_ms);
  } else
    status = 0;

  if(status != 0){
    /*
     * Set the connection_status flag to make the main thread
     * (process_dirty_connections() in serverm.c) to
     * close the connection and let the the client to try to restore
     * it since there is no other way to resynchronize the server and client.
     * If there is an error (mutex) trying to raise the flag, then
     * we try to exit the client thread entirely.
     */
    conn_element_set_fd(ce, -1);
    if(conn_element_set_connection_status(ce, -1) != 0)
      conn_element_set_exit_flag(ce);
  } else
    conn_stats_update_packets(&ce->cs, (size_t)packet_size);

  if((status == 0) && (g.f_debug != 0))
    log_msg(LOG_DEBUG, "Transmit OK to client %s", nameorip);  

  return(status);
}

/*
 * These are the functions to support the persistence of the client's
 * thread state when the connection_status flag is raised and a reconnection
 * is being waited for.
 */
static int wait_client_reconnection(struct conn_element_st *ce){
  /*
   * This function should be called if ce->fd is -1. The function sleeps
   * for ce->reconnect_wait_sleep_secs, and checks if the connection
   * was reopened. It returns the value of the flag:
   *
   * 0 => connection is not closed (reopened)
   * 1 => connection is closed
   * -1 => error from conn_element_get_closed_connection_flag() (mutex error)
   */
  int connection_status;
  int fd;
  char *nameorip;

  nameorip = conn_element_get_nameorip(ce);
  log_info("Waiting for reconnection from client %s.", nameorip); 

  sleep(ce->reconnect_wait_sleep_secs);

  connection_status = conn_element_get_connection_status(ce, &fd);
  if(connection_status == 0){
    conn_element_set_fd(ce, fd);
    log_info("Reconnected client %s.", nameorip); 
  }

  return(connection_status);
}

static int send_client1(struct conn_element_st *ce,
			void *data, uint32_t data_size){
  int protocol;
  int status = 0;

  /*
   * Find out what type of client this thread is serving and dispatch
   * accordingly.
   */
  protocol = get_client_protocol_byce(ce);

  if(protocol == PROTOCOL_NBS1)
    status = send_nbs1_client(ce, data, data_size);
  else if(protocol == PROTOCOL_NBS2)
    status = send_nbs2_client(ce, data, data_size);
  else if(protocol == PROTOCOL_EMWIN)
    status = send_emwin_client(ce, (struct emwin_queue_info_st*)data,
			       data_size);
  else {
    log_errx("Server client thread running with unknown client protocol.");
    return(1);
  }

  if(status != 0)
    conn_stats_update_errors(&ce->cs);

  return(status);
}

static int send_client(struct conn_element_st *ce,
			void *data, uint32_t data_size){

  int status = 0;
  int count = 0;

  do {
    if(conn_element_get_fd(ce) != -1)
      status = send_client1(ce, data, data_size);

    if(conn_element_get_fd(ce) != -1)
      return(status);

    /* fd == -1 */
    if(count == ce->reconnect_wait_sleep_retry)
      break;
    else
      status = wait_client_reconnection(ce);

    if(status != 0)
      ++count;

  } while((get_quit_flag() == 0) && (conn_element_get_exit_flag(ce) == 0));

  /*
   * If we are here then the connection had to be closed by one of the
   * send_xxx_client(), and the client has not reconnected within
   * the time limit set by the reconnect_wait_sleep_secs and
   * reconnect_wait_sleep_retry options.
   *
   * The caller should set the finished flag so that the thread exits
   * and the main thread deletes this entry.
   */

  return(status);
}

/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netdb.h>	/* h_errno */
#include <fcntl.h>
#include "globals.h"
#include "signal.h"	/* get/set_quit_flag() */
#include "libconnth/sock.h"
#include "libconnth/tcpsock.h"
#include "util.h"
#include "err.h"
#include "appdata.h"
#include "readn.h"
#include "slavenbs/slavenbs.h"
#include "slavefp/slavefp.h"
#include "slavein/slavein.h"
#include "slavet.h"
#include "slave.h"
#include "slave_priv.h"

static void *slavenet_main(void *arg);  /* arg is a slave_element_st */
static int slavenet_open(struct slave_element_st *slave);
static int slavenet_close(struct slave_element_st *slave);
static void slavenet_try_close(void *arg);
static int slavenet_reopen(struct slave_element_st *slave);
static int slavenet_loop(struct slave_element_st *slave);

static int slavenet_init_info(struct slave_element_st *slave);
static void slavenet_cleanup_info(void *arg);

int slavenet_spawn_thread(struct slave_element_st *slave){

  int status = 0;
  pthread_t t_id;
  pthread_attr_t attr;

  status = pthread_attr_init(&attr);

  if(status == 0)
    status = pthread_create(&t_id, &attr, slavenet_main, (void*)slave);

  if(status != 0){
    log_err("Cannot create slave thread.");
  }else{
    slave->slave_thread_id = t_id;
    slave->f_slave_thread_created = 1;
    log_info("Spawned slave thread %s:%s.",
	     slave->mastername, slave->masterport);
  }

  return(status);
}

void slavenet_kill_thread(struct slave_element_st *slave){
  /*
   * This comment is adapted from the reader.c for np channels.
   *
   * This "joins" the nbs reader thread. Normally it quits
   * in their loop when they check the quit flag. But if a reader
   * is waiting on a channel without data, it can wait for the
   * g.tmout_readbroadcast_s timeout and that may be too long.
   * Therefore we "cancel" them. It is possible that some of them
   * have already quit by the time we send them the cancelation request,
   * and in that case pthread_cancel returns ESRCH.
   */
  int status;
  void *pthread_status;

  if(slave->f_slave_thread_created == 0)
    return;

  /*
   * log_info("Canceling slave thread.");
   */
  status = pthread_cancel(slave->slave_thread_id);
  if((status != 0) && (status != ESRCH))
    log_errx("Error %d canceling slave thread %s:%s.",
	     status, slave->mastername, slave->masterport);

  status = pthread_join(slave->slave_thread_id, &pthread_status);
  if(status != 0)
    log_errx("Error %d joining slave thread.", status);
  else if(pthread_status == PTHREAD_CANCELED)
    log_info("Cancelled slave thread.");
  else if(pthread_status == NULL)
    log_info("Finished slave thread %s:%s.",
	     slave->mastername, slave->masterport);
}

static void *slavenet_main(void *arg){

  struct slave_element_st *slave = (struct slave_element_st*)arg;
  int status = 0;

  pthread_cleanup_push(slavenet_cleanup_info, arg);
  pthread_cleanup_push(slavenet_try_close, arg);

  status = slavenet_init_info(slave);
  if(status != 0)
    set_quit_flag();
  else{
    status = slavenet_open(slave);
    /*
     * If slavenet_open() returns 2, it is a configuration error and we quit
     * the application. Otherwise we assume that, if there is an error,
     * it is a temporary situation and try to reopen the connection in
     * the loop.
     */
    if(status == 2)
      set_quit_flag();
  }

  if(status == 0)
    slave_stats_connect(slave);

  while(get_quit_flag() == 0){
    /*
     * If the slave_fd is -1 it means that the socket was closed
     * due to a reading error. It could be a real error, or the server
     * not sending anything within the timeout limit and the like.
     * reopen() waits some time and tries to reopen it.
     */
    if(slave->slave_fd == -1){
      log_errx("Closed connection to %s. Reconnecting.", slave->mastername);
      status = slavenet_reopen(slave);
      if(slave->slave_fd != -1)
	slave_stats_connect(slave);
    }

    if(slave->slave_fd != -1){
      status = slavenet_loop(slave);
      /*
       * When this function returns 1 it means there was a connection error.
       * When it returns 2 it is a processing error and there is no need
       * to close the connection.
       */
      if(status == 1)
	status = slavenet_close(slave);
    }
  }

  pthread_cleanup_pop(1);
  pthread_cleanup_pop(1);

  return(NULL);
}

static int slavenet_init_info(struct slave_element_st *slave){

  int status = 0;

  if(slave->slavetype == SLAVETYPE_NBS1)
    status = slavenet_init_nbs1(slave);
  else if(slave->slavetype == SLAVETYPE_NBS2)
    status = slavenet_init_nbs2(slave);
  else{
    log_errx("Invalid value of slavetype in slavenet_init_info.");
    status = 1;
  }

  return(status);
}

static void slavenet_cleanup_info(void *arg){

  struct slave_element_st *slave = (struct slave_element_st*)arg;

  if(slave->slavetype == SLAVETYPE_NBS1)
    slavenet_cleanup_nbs1(slave);
  else if(slave->slavetype == SLAVETYPE_NBS2)
    slavenet_cleanup_nbs2(slave);
  else
    log_errx("Invalid value of slavetype in slavenet_cleanup_info.");
}

static int slavenet_open(struct slave_element_st *slave){
  /*
   * This function returns the following errors:
   *
   * -1 => operating system error
   *  1 => timed out trying to connect
   *  2 => configuration error (of masterservers)
   *
   * When the error is 1 or -1, we can assume that it is a temporary
   * situation and keep retyring. When the error is 2, we should quit
   * the application.
   */
  int status = 0;
  int gai_code;
  char *nbs_str;
  ssize_t n;
  
  if(valid_str(slave->mastername) == 0){
    log_errx("No master host defined.");
    return(2);
  }

  if(valid_str(slave->masterport) == 0){
    log_errx("Master port not set.");
    return(2);
  }

  if(slave->slavetype == SLAVETYPE_NBS1)
    nbs_str = PROTOCOL_NBS1_STR;
  else if(slave->slavetype == SLAVETYPE_NBS2)
    nbs_str = PROTOCOL_NBS2_STR;
  else{
    log_errx("Invalid slave mode in slavenet_open.");
    return(2);
  }

  slave->slave_fd = tcp_client_open_conn(slave->mastername, slave->masterport,
					 -1, slave->options.slave_so_rcvbuf,
					 &gai_code);
  if(slave->slave_fd == -1){
    status = -1;
    if(gai_code != 0)
      log_errx("Slave cannot open connection to %s. %s.",
	       slave->mastername, gai_strerror(gai_code));
    else
      log_err2("Slave cannot open connection to", slave->mastername);
  }

  if(status != 0)
    return(status);

  /* Wait at most the same amount allowed to read from the master */
  n = writen(slave->slave_fd, nbs_str, strlen(nbs_str),
	     (unsigned int)slave->options.slave_read_timeout_secs, 0);
  if(n == -1){
    status = -1;
    log_err2("Cannot communicate with", slave->mastername);
  }else if((size_t)n != strlen(nbs_str)){
    log_errx("Cannot communicate with %s. Timed out.", slave->mastername);
    status = 1;
  }

  if(status != 0){
    (void)close(slave->slave_fd);
    slave->slave_fd = -1;
  }else{
    log_info("Opened connection to %s.", slave->mastername);
  }

  return(status);
}

static int slavenet_close(struct slave_element_st *slave){

  int status = 0;

  if(slave->slave_fd == -1)
    return(0);

  status = close(slave->slave_fd);
  if(status == 0){
    slave->slave_fd = -1;
  }else
    log_err2("Error closing connection to", slave->mastername);

  return(status);
}

static void slavenet_try_close(void *arg){

  struct slave_element_st *slave = (struct slave_element_st*)arg;

  (void)slavenet_close(slave);
}
  
static int slavenet_reopen(struct slave_element_st *slave){
  /*
   * This function is meant to be used when reading from the master
   * returns an error. It closes the connection (if it is not already closed)
   * and tries to open it again after sleeping for a specified period.
   */
  int status = 0;

  status = slavenet_close(slave);
  sleep((unsigned int)slave->options.slave_reopen_timeout_secs);
  status = slavenet_open(slave);

  return(status);
}

static int slavenet_loop(struct slave_element_st *slave){

  int status = 0;

  if(slave->slavetype == SLAVETYPE_NBS1)
    status = slavenet_loop_nbs1(slave);
  else if(slave->slavetype == SLAVETYPE_NBS2)
    status = slavenet_loop_nbs2(slave);
  else
    log_errx("Invalid slavetype in slavenet_loop.");

  return(status);
}

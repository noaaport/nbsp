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
#include "slavein/slavein.h"
#include "slavet.h"
#include "slave.h"
#include "slave_priv.h"

static void *slavein_main(void *arg);  /* arg is a slave_element_st */
static int slavein_open(struct slave_element_st *slave);
static int slavein_close(struct slave_element_st *slave);
static void slavein_try_close(void *arg);
static int slavein_reopen(struct slave_element_st *slave);

static int slavein_init_info(struct slave_element_st *slave);
static void slavein_cleanup_info(void *arg);

int slavein_spawn_thread(struct slave_element_st *slave){

  int status = 0;
  pthread_t t_id;
  pthread_attr_t attr;

  status = pthread_attr_init(&attr);

  if(status == 0)
    status = pthread_create(&t_id, &attr, slavein_main, (void*)slave);

  if(status != 0){
    log_err("Cannot create slave thread.");
  }else{
    slave->slave_thread_id = t_id;
    slave->f_slave_thread_created = 1;
    log_info("Spawned slave thread %s.", slave->infifo);
  }

  return(status);
}

void slavein_kill_thread(struct slave_element_st *slave){
  /*
   * This comment is adapted from the reader.c for np channels.
   *
   * This "joins" the infifo reader thread. Normally it quits
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
    log_errx("Error %d canceling slave thread %s.", status, slave->infifo);

  status = pthread_join(slave->slave_thread_id, &pthread_status);

  if(status != 0)
    log_errx("Error %d joining slave thread %s.", status, slave->infifo);
  else if(pthread_status == PTHREAD_CANCELED)
    log_info("Cancelled slave thread %s.", slave->infifo);
  else if(pthread_status == NULL)
    log_info("Finished slave thread %s.", slave->infifo);
}

static void *slavein_main(void *arg){

  struct slave_element_st *slave = (struct slave_element_st*)arg;
  int status = 0;

  /*
   * The order is important here; the info struct must be destroyed last.
   */
  pthread_cleanup_push(slavein_cleanup_info, arg);
  pthread_cleanup_push(slavein_try_close, arg);

  status = slavein_init_info(slave);
  if(status == 0)
    status = slavein_open(slave);

  if(status != 0)
    set_quit_flag();

  while(get_quit_flag() == 0){
    /*
     * If the slave_fd is -1 it means that the socket was closed
     * due to a reading error. It could be a real error, or the server
     * not sending anything within the timeout limit and the like.
     * We wait some time and try to reopen it.
     */
    if(slave->slave_fd == -1){
      log_errx("Lost connection to %s. Trying again.", slave->infifo);
      status = slavein_reopen(slave);
    }

    if(slave->slave_fd != -1){
      status = slavein_loop(slave);
      /*
       * When this function returns 1 it means there was a reading error,
       * and we close the fifo. When it returns 2 it is a processing error
       * and there is no need to close the fifo.
       */
      if(status == 1)
	status = slavein_close(slave);
    }
  }

  pthread_cleanup_pop(1);
  pthread_cleanup_pop(1);

  return(NULL);
}

static int slavein_init_info(struct slave_element_st *slave){

  int status = 0;

  status = slavein_init(slave);

  return(status);
}

static void slavein_cleanup_info(void *arg){

  struct slave_element_st *slave = (struct slave_element_st*)arg;

  slavein_cleanup(slave);
}

static int slavein_open(struct slave_element_st *slave){

  int status = 0;

  (void)unlink(slave->infifo);
  status = mkfifo(slave->infifo, slave->options.infifo_mode);

  if(status == 0){
    /*
     * The mkfifo() sets the mode masked by the default umask. We need to call
     * chown to set the group and chmod to set the absolute mask. The
     * function chgrpmode (in libconn) does this.
     */
    status = chgrpmode(slave->infifo, slave->options.infifo_grp,
		       slave->options.infifo_mode);
  }

  /*
   * If we open the fifo O_RDONLY, then when the last writer closes the fifo
   * it will cause readn() in recv_in_packet() [reader.c] to return eof (n== 0)
   * which will trigger a return error from recv_in_packet() in slavein_loop
   * and will force to close and reopen the fifo (and the associated "error"
   * message(s). Therefore we will open the fifo RW so that the number of
   * writers never drops to zero.
   */
  if(status == 0)
    slave->slave_fd = open(slave->infifo, O_RDWR | O_NONBLOCK);

  if(slave->slave_fd == -1){
    status = -1;
    log_err2("Slave cannot open", slave->infifo);
  }

  return(status);
}

static int slavein_close(struct slave_element_st *slave){

  int status = 0;

  if(slave->slave_fd == -1)
    return(0);

  status = close(slave->slave_fd);
  if(status == 0){
    slave->slave_fd = -1;
    (void)unlink(slave->infifo);
  }else
    log_err2("Error closing", slave->infifo);

  return(status);
}

static void slavein_try_close(void *arg){

  struct slave_element_st *slave = (struct slave_element_st*)arg;

  (void)slavein_close(slave);
}
  
static int slavein_reopen(struct slave_element_st *slave){
  /*
   * This function is meant to be used when reading from the master
   * returns an error. It closes the connection and tries to open it
   * again. If it fails, then it sleeps for a time out period before
   * returning.
   */
  int status = 0;

  status = slavein_close(slave);
  if(status == 0)
    status = slavein_open(slave);

  if(status != 0)
    sleep((unsigned int)slave->options.slave_reopen_timeout_secs);

  return(status);
}

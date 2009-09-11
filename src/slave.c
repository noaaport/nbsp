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
#include "libconnth/sock.h"
#include "libconnth/tcpsock.h"
#include "globals.h"
#include "util.h"
#include "err.h"
#include "appdata.h"
#include "readn.h"
#include "slave.h"
#include "slavenbs/slavenbs.h"
#include "slavefp/slavefp.h"
#include "slavein/slavein.h"

static int slave_net_open(void);
static int slave_net_close(void);
static int slave_infeed_open(void);
static int slave_infeed_close(void);

int spawn_slave(void){

  int status = 0;

  if(feedmode_slavenet_nbs1_enabled())
    status = spawn_slavenbs();
  else if(feedmode_slavenet_nbs2_enabled())
    status = spawn_slavefp();
  else if(feedmode_slaveinfeed_enabled())
    status = spawn_slavein();
  else{
    status = 1;
    log_errx("Invalid configuration value of slavemode.");
  }

  return(status);
}

int spawn_slave_thread(void *(*thread_main)(void*)){

  int status = 0;
  pthread_t t_id;
  pthread_attr_t attr;

  status = pthread_attr_init(&attr);
  if(status == 0)
    status = pthread_create(&t_id, &attr, thread_main, NULL);

  if(status != 0){
    log_err("Cannot create slave thread.");
  }else{
    g.slave_thread_id = t_id;
    g.f_slave_thread_created = 1;

    log_info("Spawned slave thread.");
  }

  return(status);
}

void kill_slave_thread(void){
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

  if(g.f_slave_thread_created == 0)
    return;

  /*
   * log_info("Canceling slave thread.");
   */
  status = pthread_cancel(g.slave_thread_id);
  if((status != 0) && (status != ESRCH))
    log_errx("Error %d canceling slave thread.", status);

  status = pthread_join(g.slave_thread_id, &pthread_status);
  if(status != 0)
    log_errx("Error %d joining slave thread.", status);
  else if(pthread_status == PTHREAD_CANCELED)
    log_info("Cancelled slave thread.");
  else if(pthread_status == NULL)
    log_info("Finished slave thread.");
}

int slave_open(void){

  int status = 0;
  
  if(feedmode_slavenet_nbs1_enabled() || feedmode_slavenet_nbs2_enabled())
    status = slave_net_open();
  else if(feedmode_slaveinfeed_enabled())
    status = slave_infeed_open();
  else{
    status = 1;
    log_errx("Invalid configuration value of slavemode.");
  }

  return(status);
}

int slave_close(void){

  int status = 0;

  if(feedmode_slavenet_nbs1_enabled() || feedmode_slavenet_nbs2_enabled())
    status = slave_net_close();
  else if(feedmode_slaveinfeed_enabled())
    status = slave_infeed_close();

  return(status);
}

static int slave_net_open(void){

  int status = 0;
  int gai_code;
  char *nbs_str;
  ssize_t n;
  
  if(valid_str(g.mastername) == 0){
    log_errx("No master host defined.");
    return(1);
  }

  if(valid_str(g.masterport) == 0){
    log_errx("Master port not set.");
    return(1);
  }

  if(feedmode_slavenet_nbs1_enabled())
    nbs_str = PROTOCOL_NBS1_STR;
  else if(feedmode_slavenet_nbs2_enabled())
    nbs_str = PROTOCOL_NBS2_STR;
  else{
    log_errx("Invalid slave mode.");
    return(1);
  }

  g.slave_fd = tcp_client_open_conn(g.mastername, g.masterport,
				    -1, g.slave_so_rcvbuf, &gai_code);
  if(g.slave_fd == -1){
    status = -1;
    if(gai_code != 0)
      log_errx("Slave cannot open connection to %s. %s.",
	       g.mastername, gai_strerror(gai_code));
    else
      log_err2("Slave cannot open connection to", g.mastername);
  }

  if(status != 0)
    return(status);

  /* Wait at most the same amount allowed to read from the master */
  n = writen(g.slave_fd, nbs_str, strlen(nbs_str),
	     (unsigned int)g.slave_read_timeout_s, 0);
  if(n == -1){
    status = -1;
    log_err2("Cannot communicate with", g.mastername);
  }else if((size_t)n != strlen(nbs_str)){
    log_errx("Cannot communicate with %s. Timed out.", g.mastername);
    status = 1;
  }

  if(status != 0){
    close(g.slave_fd);
    g.slave_fd = -1;
  }else{
    log_info("Opened connection to %s.", g.mastername);
  }

  return(status);
}

static int slave_net_close(void){

  int status = 0;

  if(g.slave_fd != -1){
    status = close(g.slave_fd);
    g.slave_fd = -1;
  }

  if(status == -1)
    log_err2("Error closing connection to", g.mastername);

  return(status);
}

static int slave_infeed_open(void){

  int status = 0;

  (void)unlink(g.infifo);
  status = mkfifo(g.infifo, g.infifo_mode);

  if(status == 0){
    /*
     * The mkfifo() sets mode masked by the default umask. We need to call
     * chown to set the group and chmod to set the absolute mask. The
     * function chgrpmode (in libconn) does this.
     */
    status = chgrpmode(g.infifo, g.infifo_grp, g.infifo_mode);
  }

  if(status == 0)
    g.slave_fd = open(g.infifo, O_RDONLY | O_NONBLOCK);

  if(g.slave_fd == -1){
    status = -1;
    log_err2("Slave cannot open", g.infifo);
  }

  return(status);
}

static int slave_infeed_close(void){

  int status = 0;

  if(g.slave_fd != -1){
    status = close(g.slave_fd);
    g.slave_fd = -1;
  }

  (void)unlink(g.infifo);

  if(status == -1)
    log_err2("Error closing", g.infifo);

  return(status);
}

int slave_reopen(void){
  /*
   * This function is meant to be used when reading from the master
   * returns an error. It closes the connection and tries to open it
   * again. If it fails, then it sleeps for a time out period before
   * returning.
   */
  int status = 0;

  status = slave_close();
  if(status == 0)
    status = slave_open();

  if(status != 0)
    sleep((unsigned int)g.slave_reopen_timeout_s);

  return(status);
}

/*
 * These functions should used instead of directly checking
 * g.feedmode.
 */
int feedmode_slavenet_nbs1_enabled(void){

  if(g.feedmode == FEEDMODE_SLAVENET_NBS1)
    return(1);

  return(0);
}

int feedmode_slavenet_nbs2_enabled(void){

  if(g.feedmode == FEEDMODE_SLAVENET_NBS2)
    return(1);

  return(0);
}

int feedmode_slaveinfeed_enabled(void){

  if((g.feedmode == FEEDMODE_SLAVEINFEED) ||
     (g.feedmode == FEEDMODE_MASTER_INFEED))
    return(1);

  return(0);
}

int feedmode_master_enabled(void){

  if((g.feedmode == FEEDMODE_MASTER) || (g.feedmode == FEEDMODE_MASTER_INFEED))
    return(1);

  return(0);
}

int feedmode_slave_enabled(void){

  if(g.feedmode != FEEDMODE_MASTER)
    return(1);

  return(0);
}

/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <pthread.h>
#include <errno.h>
#include "../slave.h"
#include "../err.h"
#include "../signal.h"
#include "../globals.h"
#include "../packfp.h"
#include "slavefp.h"
#include "framep.h"

/*
 * static variables
 */
static struct packet_info_st gpacketinfo = {0, 0, 0, 0, 0,
					    NULL, NULL, NULL, 0};

static int init_slavefp(void);
static void *slavefp_main(void *arg);
static int slavefp_loop(void);

int spawn_slavefp(void){

  int status = 0;

  status = init_slavefp();

  if(status == 0)
    status = spawn_slave_thread(slavefp_main);

  return(status);
}

static void *slavefp_main(void *arg __attribute__((unused))){

  int status = 0;

  while(get_quit_flag() == 0){
    status = slavefp_loop();
  }

  nbsfp_packetinfo_destroy_pool(&gpacketinfo);

  return(NULL);
}

static int slavefp_loop(void){
  /*
   * After a reading error, it is best to close and reopen the connection.
   */
  int status = 0;
  int cancel_state;

  pthread_testcancel();

  if(g.slave_fd == -1){
    log_errx("Lost connection to %s.", g.mastername);
    status = slave_reopen();
  }

  if(status != 0)
    return(status);

  status = recv_fp_packet(g.slave_fd, &gpacketinfo,
			  (unsigned int)g.slave_read_timeout_s,
			  g.slave_read_timeout_retry);

  if(status == -1){
    log_err2("Error reading from master", g.mastername);
  }else if(status == 1){
    log_info("Timed out reading from master %s", g.mastername);
  }else if(status == 2){
    log_errx("Corrupt packet reading from %s", g.mastername);
  }

  if(status != 0)
    (void)slave_close();

  if(status == 0){
    (void)pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cancel_state);
    status = slavefpproc(&gpacketinfo);
    (void)pthread_setcancelstate(cancel_state, &cancel_state);
  }

  return(status);
}

static int init_slavefp(void){

  if(nbsfp_packetinfo_init_pool(&gpacketinfo) != 0){
    log_err("Cannot initalize the receive packetinfo memory pool.");
    return(-1);
  }

  return(0);
}

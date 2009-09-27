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

static int slavefp_packetinfo_create(struct packet_info_st **packetinfo);
static void slavefp_packetinfo_destroy(struct packet_info_st *packetinfo);

static int slavefp_packetinfo_create(struct packet_info_st **packetinfo){

  struct packet_info_st *p;
  int status;

  status = nbsfp_packetinfo_create(&p);
  if(status != 0){
    log_err("Cannot initalize the slavefp packetinfo.");
  }else{
    *packetinfo = p;
  }

  return(status);
}

static void slavefp_packetinfo_destroy(struct packet_info_st *packetinfo){

    nbsfp_packetinfo_destroy(packetinfo);
}

int slavenet_init_nbs2(struct slave_element_st *slave){

  struct packet_info_st *packetinfo;  
  int status;

  status = slavefp_packetinfo_create(&packetinfo);
  if(status == 0)
    slave->info = (void*)packetinfo;

  return(status);
}

void slavenet_cleanup_nbs2(struct slave_element_st *slave){

  if(slave->info == NULL)
    return;

  slavefp_packetinfo_destroy((struct packet_info_st*)slave->info);
  slave->info = NULL;
}

int slavenet_loop_nbs2(struct slave_element_st *slave){
  /*
   * After a reading error, it is best to close and reopen the connection.
   * This function returns 1 when there is such an error, or 2 when
   * there is a processig error, or 0 when there are no errors.
   */
  int status = 0;
  int cancel_state;

  pthread_testcancel();

  status = recv_fp_packet(slave->slave_fd,
			  (struct packet_info_st*)slave->info,
			  (unsigned int)slave->options.slave_read_timeout_secs,
			  slave->options.slave_read_timeout_retry);

  if(status == -1){
    log_err2("Error reading from master", slave->mastername);
  }else if(status == 1){
    log_info("Timed out reading from master %s", slave->mastername);
  }else if(status == 2){
    log_errx("Corrupt packet reading from %s", slave->mastername);
  }

  /*
   * If there is an error at this stage, the application should close
   * the connection and try to reopen it.
   */
  if(status != 0){
    slave_stats_update_connect_errors(slave);
    return(1);
  }

  (void)pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cancel_state);
  status = slavefpproc((struct packet_info_st*)slave->info);
  (void)pthread_setcancelstate(cancel_state, &cancel_state);

  if(status != 0){
    slave_stats_update_errors(slave);
    return(2);
  }

  slave_stats_update_packets(slave,
	      ((struct packet_info_st*)slave->info)->packet_size);
  slave_stats_report(slave);

  return(status);
}

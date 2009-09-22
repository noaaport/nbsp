/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "../slave.h"
#include "../err.h"
#include "../signal.h"
#include "../globals.h"
#include "slavenbs.h"
#include "framep.h"

int slavenet_init_nbs1(struct slave_element_st *
		       slave __attribute__ ((unused))){

  /*
   * Nothing to do.
   */

  return(0);
}

void slavenet_cleanup_nbs1(struct slave_element_st *
			   slave __attribute__ ((unused))){

  /*
   * Nothing to do.
   */

  return;
}

int slavenet_loop_nbs1(struct slave_element_st *slave){
  /*
   * After a reading error, it is best to close and reopen the connection.
   * This function returns 1 when there is such an error, or 2 when
   * there is a processig error, or 0 when there are no errors.
   */
  int status = 0;
  int cancel_state;
  struct nbs1_packet_st nbs;

  pthread_testcancel();

  status = recv_nbs_packet(slave->slave_fd, &nbs,
			   (unsigned int)slave->options.slave_read_timeout_s,
			   slave->options.slave_read_timeout_retry);
  /*
   * Set this thread's channel into the pctl table.
   */
  nbs.slavenbs_reader_index = slave->slavenbs_reader_index;

  if(status == -1)
    log_err2("Error reading from master", slave->mastername);
  else if(status == -2)
    log_errx("Timed out receiving from %s", slave->mastername);
  else if(status == -3)
    log_info("Connection closed receiving from %s", slave->mastername);
  else if(status == 1)
    log_info("Timed out or connection closed reading from %s",
	     slave->mastername);
  else if(status == 2)
    log_errx("Corrupt packet reading from %s", slave->mastername);

  /*
   * If there is an error at this stage, the application should close
   * the connection and try to reopen it. Return 1 in this case.
   */
  if(status != 0){
    slave_stats_update_connect_errors(slave);
    return(1);
  }

  (void)pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cancel_state);
  status = slavenbsproc(&nbs);
  (void)pthread_setcancelstate(cancel_state, &cancel_state);

  /* The application should not close the connection in this case. */
  if(status != 0){
    slave_stats_update_errors(slave);
    return(2);
  }

  slave_stats_update_packets(slave, nbs.packet_size);
  slave_stats_report(slave);

  return(0);
}

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

static void *slavenbs_main(void *arg);
static int slavenbs_loop(void);

int spawn_slavenbs(void){

  int status = 0;

  status = spawn_slave_thread(slavenbs_main);

  return(status);
}

static void *slavenbs_main(void *arg __attribute__((unused))){

  int status = 0;

  while(get_quit_flag() == 0){
    status = slavenbs_loop();
  }

  return(NULL);
}

static int slavenbs_loop(void){
  /*
   * After a reading error, it is best to close and reopen the connection.
   */
  int status = 0;
  int cancel_state;
  struct nbs1_packet_st nbs;

  pthread_testcancel();

  /*
   * If the slave_fd is -1 it means that the socket was closed
   * due to a reading error. It could be a real error, or the server
   * not sending anything within the timeout limit and the like.
   * We wait some time and try to reopen it.
   */
  if(g.slave_fd == -1){
    log_errx("Lost connection to %s. Trying again.", g.mastername);
    sleep((unsigned int)g.slave_reopen_timeout_s);
    status = slave_reopen();
  }

  if(status != 0)
    return(status);

  status = recv_nbs_packet(g.slave_fd, &nbs,
			   (unsigned int)g.slave_read_timeout_s,
			   g.slave_read_timeout_retry);
  /*
   * At present there is only one reader in the slave.
   */
  nbs.slavenbs_reader_index = 0;

  if(status == -1)
    log_err2("Error reading from master", g.mastername);
  else if(status == -2)
    log_errx("Timed out receiving from %s", g.mastername);
  else if(status == -3)
    log_info("Connection closed receiving from %s", g.mastername);
  else if(status == 1)
    log_info("Timed out or connection closed reading from %s", g.mastername);
  else if(status == 2)
    log_errx("Corrupt packet reading from %s", g.mastername);

  if(status != 0)
    (void)slave_close();

  if(status == 0){
    (void)pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cancel_state);
    status = slavenbsproc(&nbs);
    (void)pthread_setcancelstate(cancel_state, &cancel_state);
  }

  return(status);
}

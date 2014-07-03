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
#include <stdlib.h>
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

static int slave_spawn_thread(struct slave_element_st *slave);
static void slave_kill_thread(struct slave_element_st *slave);
static char *slave_make_configuration_string(void);

/*
 * These functions should used instead of directly checking
 * g.feedmode.
 */
int feedmode_noaaport_enabled(void){

  if((g.feedmode & FEEDMODE_NOAAPORT) != 0)
    return(1);

  return(0);
}

int feedmode_masterservers_enabled(void){

  if((g.feedmode & FEEDMODE_MASTERSERVERS) != 0)
    return(1);

  return(0);
}

int feedmode_inputfifo_enabled(void){

  if((g.feedmode & FEEDMODE_INPUTFIFO) != 0)
    return(1);

  return(0);
}

int feedmode_slave_enabled(void){

  if(feedmode_inputfifo_enabled() || feedmode_masterservers_enabled())
    return(1);

  return(0);
}

int feedmode_slave_nbs1_enabled(void){
  /*
   * This function can be called only after the slave table has been
   * initialized, so that the number of nbs1 readers has been determined.
   */
  if(feedmode_masterservers_enabled() && (g.slavet->num_slavenbs_readers != 0))
    return(1);

  return(0);
}

/*
 * These are the functions called by main to launch and terminate
 * the slave threads.
 */
int init_slavet(void){

  struct slave_table_st *slavet;
  struct slave_options_st defaults;
  char *conf_str = NULL;
  int status = 0;

  defaults.infifo_mode = g.infifo_mode;
  defaults.infifo_grp = g.infifo_grp;
  defaults.slavestatsfile = g.slavestatsfile;
  defaults.slave_masterport = g.slave_masterport;
  defaults.slave_read_timeout_secs = g.slave_read_timeout_secs;
  defaults.slave_read_timeout_retry = g.slave_read_timeout_retry;
  defaults.slave_reopen_timeout_secs = g.slave_reopen_timeout_secs;
  defaults.slave_so_rcvbuf = g.slave_so_rcvbuf;
  defaults.slave_stats_logperiod_secs = g.slave_stats_logperiod_secs;

  /*
   * Check both of these here once and for all
   */
  if(feedmode_masterservers_enabled() && (valid_str(g.masterservers) == 0)){
    log_errx("Slave mode enabled with masterservers unset.");
    return(-1);
  }

  if(feedmode_inputfifo_enabled() && (valid_str(g.infifo) == 0)){
    log_errx("Input fifo mode enabled with invalid setting of infifo.");
    return(-1);
  }

  if(feedmode_inputfifo_enabled()){
    conf_str = slave_make_configuration_string();
    if(conf_str == NULL)
      return(-1);

    status = slave_table_create(&slavet, conf_str, &defaults);
    free(conf_str);
  }else
    status = slave_table_create(&slavet, g.masterservers, &defaults);

  if(status == 0)
    g.slavet = slavet;

  return(status);
}

void cleanup_slavet(void){

  if(g.slavet == NULL)
    return;

  slave_table_destroy(g.slavet);
  g.slavet = NULL;
}

int spawn_slave_threads(void){

  int status = 0;
  int i;

  for(i = 0; i < g.slavet->numslaves; ++i){
    status = slave_spawn_thread(&g.slavet->slave[i]);

    if(status != 0)
      break;
  }

  return(status);
}

void kill_slave_threads(void){

  int i;

  if(g.slavet == NULL)
    return;

  for(i = 0; i < g.slavet->numslaves; ++i){
    slave_kill_thread(&g.slavet->slave[i]);
  }
}

static int slave_spawn_thread(struct slave_element_st *slave){

  int status = 0;

  if(slave->slavetype == SLAVETYPE_NBS1)
    status = slavenet_spawn_thread(slave);
  else if(slave->slavetype == SLAVETYPE_NBS2)
    status = slavenet_spawn_thread(slave);
  else if (slave->slavetype == SLAVETYPE_INFIFO)
    status = slavein_spawn_thread(slave);
  else{
    log_errx("Invalid value of slavetype in slave_spawn_thread.");
    status = 1;
  }

  return(status);
}

static void slave_kill_thread(struct slave_element_st *slave){

  if(slave->slavetype == SLAVETYPE_NBS1)
    slavenet_kill_thread(slave);
  else if(slave->slavetype == SLAVETYPE_NBS2)
    slavenet_kill_thread(slave);
  else if (slave->slavetype == SLAVETYPE_INFIFO)
    slavein_kill_thread(slave);
  else
    log_errx("Invalid value of slavetype in slave_kill_thread.");
}

static char *slave_make_configuration_string(void){
  /*
   * This function should be called after calling
   * feedmode_inputfifo_enabled(), and if that function returns 1.
   * It will then add the infifo configuration to the masterservers list.
   */
  char *r;
  int n;

  if(valid_str(g.infifo) == 0){
    /*
     * This should have been caught already by the caller of this function
     */
    log_errx("Invalid setting of infifo.");
    return(NULL);
  }

  /* Leave room for 3, */
  n = strlen(g.infifo) + 1 + strlen(SLAVE_STRING_JOIN2);

  if(feedmode_masterservers_enabled()){
    if(valid_str(g.masterservers) == 0){
      /*
       * This should have been caught already by the caller of this function
       */
      log_errx("Invalid setting of masterservers.");
      return(NULL);
    }
    /* Leave room for the SEP1 char */
    n += strlen(g.masterservers) + strlen(SLAVE_STRING_JOIN1);
  }

  r = malloc(n + 1);
  if(r == NULL)
    return(NULL);

  if(feedmode_masterservers_enabled()){
    if(snprintf(r, n + 1, "3%s%s%s%s",
		SLAVE_STRING_JOIN2, g.infifo,
		SLAVE_STRING_JOIN1, g.masterservers) != n){
      log_errx("Configuration error in slave_make_configuration_string().");
      free(r);
      return(NULL);
    }
  }else{
    if(snprintf(r, n + 1, "3%s%s", SLAVE_STRING_JOIN2, g.infifo) != n){
      log_errx("Configuration error in slave_make_configuration_string().");
      free(r);
      return(NULL);
    }
  }

  return(r);
}

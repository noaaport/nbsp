/*
 * Copyright (c) 2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <sys/types.h>
#include <stdio.h>
#include "cs.h"

void conn_stats_init(struct conn_stats_st *cs){

  cs->ctime = time(NULL);
  cs->errors_ctime = 0;
  cs->packets_ctime = 0;
  cs->bytes_ctime = 0.0;
  cs->rtime = time(NULL);
  cs->errors_rtime = 0;
  cs->packets_rtime = 0;
  cs->bytes_rtime = 0.0;
}

void conn_stats_reset(struct conn_stats_st *cs){

  cs->rtime = time(NULL);
  cs->errors_rtime = 0;
  cs->packets_rtime = 0;
  cs->bytes_rtime = 0.0;
}

void conn_stats_update_packets(struct conn_stats_st *cs, size_t packet_size){

  ++cs->packets_rtime;
  cs->bytes_rtime += (double)packet_size;

  ++cs->packets_ctime;
  cs->bytes_ctime += (double)packet_size;
}

void conn_stats_update_errors(struct conn_stats_st *cs){

  ++cs->errors_rtime;
  ++cs->errors_ctime;
}

time_t conn_stats_get_ctime(struct conn_stats_st *cs){

  return(cs->ctime);
}

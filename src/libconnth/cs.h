/*
 * Copyright (c) 2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef LIBCONNTH_CS_H
#define LIBCONNTH_CS_H

#include <time.h>

struct conn_stats_st {
  time_t ctime;			/* connection time */
  unsigned int errors_ctime;	/* since connection time to reset time */
  unsigned int packets_ctime;
  double bytes_ctime;
  time_t rtime;			/* time of last reset */
  unsigned int errors_rtime;	/* errors since reset time */
  unsigned int packets_rtime;	/* packets received since reset time */
  double bytes_rtime;
};

void conn_stats_init(struct conn_stats_st *cs);
void conn_stats_reset(struct conn_stats_st *cs);
void conn_stats_update_packets(struct conn_stats_st *cs, size_t packet_size);
void conn_stats_update_errors(struct conn_stats_st *cs);

time_t conn_stats_get_ctime(struct conn_stats_st *cs);

#endif

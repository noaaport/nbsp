/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef STATS_H
#define STATS_H

#include <time.h>

struct nbsp_stats_st {
  unsigned int frames_processed;
  unsigned int frames_jumps;		/* sequence jumps */	
  unsigned int frames_bad;
  unsigned int frames_received;
  unsigned int frames_data_size;
  unsigned int products_transmitted;
  unsigned int products_retransmitted;
  unsigned int products_retransmitted_c; /* retrans. and received complete */
  unsigned int products_retransmitted_p; /* retransmitted  and processed */
  unsigned int products_retransmitted_i; /* retransmitted  but ignored  */
  unsigned int products_retransmitted_r; /* number of missed recovered later */
  unsigned int products_completed;	/* all frames received */
  unsigned int products_missed;		/* missing frames */
  unsigned int products_bad;		/* processed with errors */
  unsigned int products_aborted;
  unsigned int products_rejected;	/* rejected by filter */
  unsigned int products_rtx_index;	/* rtx per 1000 products */
  int f_valid;
  time_t time;
};

void nbspstats_init(void);
void nbspstats_update(void);
void nbspstats_report(char *fname);

unsigned int nbspstats_get_rtx_index(void);

void update_stats_frames_received(unsigned int frame_data_size);
void update_stats_frames(int status);
void update_stats_frames_jumps(void);
void update_stats_products(int status);
void update_stats_products_transmitted(void);
void update_stats_products_retransmitted(void);
void update_stats_products_retransmitted_c(void);
void update_stats_products_retransmitted_pi(int orig_status);
void update_stats_products_recovered(void);
void update_stats_products_completed(void);
void update_stats_products_missed(void);
void update_stats_products_aborted(void);
void update_stats_products_rejected(void);

#endif

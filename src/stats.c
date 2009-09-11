/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdio.h>
#include "err.h"
#include "stats.h"
#include "globals.h"
#include "defaults.h"

static void init_counters(void);
static void update_stats_goodframes(void);
static void update_stats_badframes(void);
static void update_stats_badproducts(void);
static void update_stats_rtx_index(void);

void nbspstats_init(void){

  init_counters();
  g.nbspstats.products_rtx_index = 0;
  g.nbspstats.time = time(NULL);
}
  
static void init_counters(void){

  g.nbspstats.frames_processed = 0;
  g.nbspstats.frames_jumps = 0;	
  g.nbspstats.frames_bad = 0;
  g.nbspstats.frames_received = 0;
  g.nbspstats.frames_data_size = 0;
  g.nbspstats.products_transmitted = 0;
  g.nbspstats.products_retransmitted = 0;	/* detected by readers */
  g.nbspstats.products_retransmitted_c = 0;	/* received complete */
  g.nbspstats.products_retransmitted_p = 0;	/* processed */
  g.nbspstats.products_retransmitted_i = 0;	/* ignored */
  g.nbspstats.products_retransmitted_r = 0;	/* recovered */
  g.nbspstats.products_completed = 0;
  g.nbspstats.products_missed = 0;
  g.nbspstats.products_bad = 0;
  g.nbspstats.products_aborted = 0;
  g.nbspstats.products_rejected = 0;
};

void nbspstats_update(void){

  time_t now;
  
  now = time(NULL);

  /*
   * This function is now called from the periodic module, and therefore
   * this should not be used.
   *
   *  if(difftime(now, g.nbspstats.time) < (double)g.nbspstats_logperiod_s)
   *    return;
   */

  update_stats_rtx_index();
  g.nbspstats.time = now;
  nbspstats_report(g.statusfile);
  init_counters();  /* does not clear rtx_index */
}

void nbspstats_report(char *fname){

  FILE *f;

  f = fopen(fname, "a");
  if(f == NULL){
    log_err2("Could not open", fname);
    return;
  }

  fprintf(f, "%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
	  (unsigned int)g.nbspstats.time,
	  g.nbspstats.frames_received,
	  g.nbspstats.frames_processed,
	  g.nbspstats.frames_jumps,
	  g.nbspstats.frames_bad,
	  g.nbspstats.frames_data_size,
	  g.nbspstats.products_transmitted,
	  g.nbspstats.products_completed,
	  g.nbspstats.products_missed,
	  g.nbspstats.products_retransmitted,
	  g.nbspstats.products_retransmitted_c,
	  g.nbspstats.products_retransmitted_p,
	  g.nbspstats.products_retransmitted_i,
	  g.nbspstats.products_retransmitted_r,
	  g.nbspstats.products_rejected,
	  g.nbspstats.products_aborted,
	  g.nbspstats.products_bad);

  fclose(f);
}

void update_stats_frames_received(unsigned int frame_data_size){

  ++g.nbspstats.frames_received;
  g.nbspstats.frames_data_size += frame_data_size;
}

void update_stats_frames(int status){
  /*
   * Note: For updating missing frames the function
   * update_stats_frames_jumps() must be called separately.
   */
  if(status == 0)
    update_stats_goodframes();
  else
    update_stats_badframes();
}

void update_stats_frames_jumps(void){

  ++g.nbspstats.frames_jumps;
}

void update_stats_products(int status){

  if(status != 0)
    update_stats_badproducts();
}

void update_stats_products_transmitted(void){

  ++g.nbspstats.products_transmitted;
}

void update_stats_products_retransmitted(void){
  /*
   * This is the number of transmissions that had the retransmission
   * flag set. This number is increased indepdendently of whether the
   * entire product is received or not.
   */
  ++g.nbspstats.products_retransmitted;
}

void update_stats_products_retransmitted_c(void){
  /*
   * This number is increased only if the entire product is received.
   */
  ++g.nbspstats.products_retransmitted_c;
}

void update_stats_products_retransmitted_pi(int orig_status){
  /*
   * This function keeps track of the numbers of the retransmitted
   * products that were ignored, and the number that were processed.
   * These counters are increased only if the entire product has been
   * received.
   */

  if(orig_status == 0)
    ++g.nbspstats.products_retransmitted_i;
  else
    ++g.nbspstats.products_retransmitted_p;
}

void update_stats_products_recovered(void){
  /*
   * This is the number of retransmitted products that were able
   * to recover a missing one due missing frames.
   */
  ++g.nbspstats.products_retransmitted_r;
}

void update_stats_products_missed(void){

  ++g.nbspstats.products_missed;
}

void update_stats_products_completed(void){
  /*
   * All frames received.
   */
  ++g.nbspstats.products_completed;
}

void update_stats_products_aborted(void){

  ++g.nbspstats.products_aborted;
}

void update_stats_products_rejected(void){

  ++g.nbspstats.products_rejected;
}
    
static void update_stats_goodframes(void){

  ++g.nbspstats.frames_processed;
}

static void update_stats_badframes(void){

  ++g.nbspstats.frames_bad;
}

static void update_stats_badproducts(void){

  ++g.nbspstats.products_bad;
}

static void update_stats_rtx_index(void){
  /*
   * The rtx index is the number of retransmissions detected per 1000
   * products.
   */
  unsigned int rtxindex;

  if(g.nbspstats.products_transmitted != 0){
    rtxindex = (g.nbspstats.products_retransmitted * 1000) /
      g.nbspstats.products_transmitted;
    g.nbspstats.products_rtx_index = rtxindex;
  }
}

unsigned int nbspstats_get_rtx_index(void){

  return(g.nbspstats.products_rtx_index);
}

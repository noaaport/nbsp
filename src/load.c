/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdlib.h>
#include <unistd.h>
#include "err.h"
#include "globals.h"
#include "stats.h"
#include "load.h"

void verify_load_ave_condition(void){
  /*
   * The high load retransmission flag is checked when either the
   * g.max_load_ave_soft or g.max_load_ave_hard exceed their value.
   */
  double loadavg[3];
  int n;
  int current_load_avg;

  g.f_max_load_ave = 0;
  g.f_max_load_rtx = 0;

  n = getloadavg(loadavg, 3);
  if(n == -1){
    log_err("Error from getloadavg().");
    return;
  }
  current_load_avg = (int)loadavg[0];

  if((g.max_load_ave_soft > 0) && (current_load_avg >= g.max_load_ave_soft))
    g.f_max_load_ave = 1;

  if((g.max_load_ave_hard > 0) && (current_load_avg >= g.max_load_ave_hard))
    g.f_max_load_ave = 2;

  if((g.f_max_load_ave > 0) && (g.max_load_rtx_index > 0) &&
     (nbspstats_get_rtx_index() >= (unsigned int)g.max_load_rtx_index)){
    g.f_max_load_rtx = 1;
  }
}

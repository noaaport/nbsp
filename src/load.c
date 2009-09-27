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

  g.f_loadave_max = 0;
  g.f_loadave_max_rtx = 0;

  n = getloadavg(loadavg, 3);
  if(n == -1){
    log_err("Error from getloadavg().");
    return;
  }
  current_load_avg = (int)loadavg[0];

  if((g.loadave_max_soft > 0) && (current_load_avg >= g.loadave_max_soft))
    g.f_loadave_max = 1;

  if((g.loadave_max_hard > 0) && (current_load_avg >= g.loadave_max_hard))
    g.f_loadave_max = 2;

  if((g.f_loadave_max > 0) && (g.loadave_max_rtx_index > 0) &&
     (nbspstats_get_rtx_index() >= (unsigned int)g.loadave_max_rtx_index)){
    g.f_loadave_max_rtx = 1;
  }
}

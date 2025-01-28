/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "dcgoesr_cmilevel.h"

void cmilevel(struct goesr_st *gp) {
  /*
   * This function calculates the "normalized" cmi.
   */
  int k;
  double cmi_max, cmi_min, norm, cmi_normalized;
  double *cmi = gp->cmi;
  int Npoints = gp->Npoints;

  /* determine the max and min */
  cmi_max = 0.0;
  cmi_min = FLT_MAX;
    
  for(k = 0; k < Npoints; ++k) {
    if(cmi[k] > cmi_max)
      cmi_max = cmi[k];

    if(cmi[k] < cmi_min)
      cmi_min = cmi[k];
  }

  /* determine the normalized values */
  norm = 255.0/(cmi_max - cmi_min);
  
  for(k = 0; k < Npoints; ++k) {
    cmi_normalized = (cmi[k] - cmi_min) * norm;
    gp->pmap.points[k].level = (uint8_t)cmi_normalized;
  }
}

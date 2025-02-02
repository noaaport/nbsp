/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdlib.h>
#include <stdio.h>
#include "err.h"
#include "dcgoesr_nc.h"

int dcgoesr_info_write(char *file, struct goesr_st *goesr){

  FILE *f;
  int n;

  f = fopen(file, "w");
  if(f == NULL){
    log_err(1, "Cannot open %", file);
    return(1);
  }

  n = fprintf(f, "nx: %d\n", goesr->nx);

  if(n > 0)
    n = fprintf(f, "ny: %d\n", goesr->ny);

  if(n > 0)
    n = fprintf(f, "bb: %f %f %f %f\n",
		goesr->pmap.lon_min, goesr->pmap.lat_min,
		goesr->pmap.lon_max, goesr->pmap.lat_max);


  /*
   * This is the "maximum enclosing rectangle"
   */
  if(n > 0)
    n = fprintf(f, "llur: %f %f %f %f\n",
		goesr->pmap.lon_ll, goesr->pmap.lat_ll,
		goesr->pmap.lon_ur, goesr->pmap.lat_ur);

  fclose(f);

  if(n < 0){
    log_err(1, "Error writing info file.");
    return(1);
  }

  return(0);
}

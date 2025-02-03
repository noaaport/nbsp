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

int dcgoesr_info_write(char *file, struct dcgoesr_point_map_st *pm) {

  FILE *f;
  int n;

  f = fopen(file, "w");
  if(f == NULL){
    log_err(0, "Cannot open %", file);
    return(1);
  }

  n = fprintf(f, "nx: %d\n", pm->nx);

  if(n > 0)
    n = fprintf(f, "ny: %d\n", pm->ny);

  if(n > 0)
    n = fprintf(f, "bb: %f %f %f %f\n",
		pm->lon_min, pm->lat_min,
		pm->lon_max, pm->lat_max);


  /*
   * This is the "maximum enclosing rectangle"
   */
  if(n > 0)
    n = fprintf(f, "llur: %f %f %f %f\n",
		pm->lon_ll, pm->lat_ll,
		pm->lon_ur, pm->lat_ur);

  fclose(f);

  if(n < 0){
    log_err(0, "Error writing info file.");
    return(1);
  }

  return(0);
}

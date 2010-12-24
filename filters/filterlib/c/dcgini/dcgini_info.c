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
#include "dcgini.h"

int dcgini_info_write(char *file, struct dcgini_st *dcg){

  FILE *f;
  int n;

  f = fopen(file, "w");
  if(f == NULL){
    log_err(1, "Cannot open %", file);
    return(1);
  }

  n = fprintf(f, "seconds: %" PRIuMAX "\n",
	      (uintmax_t)dcg->pdb.unixseconds);
  if(n > 0)
    n = fprintf(f, "sector: %d\n", dcg->pdb.sector);

  if(n > 0)
    n = fprintf(f, "channel: %d\n", dcg->pdb.channel);

  if(n > 0)
    n = fprintf(f, "resolution: %d\n", dcg->pdb.res);

  if(n > 0)
    n = fprintf(f, "bb: %f %f %f %f\n",
		dcg->pointmap.lon_min, dcg->pointmap.lat_min,
		dcg->pointmap.lon_max, dcg->pointmap.lat_max);

  if(n > 0)
    n = fprintf(f, "size: %d %d\n", dcg->pdb.nx, dcg->pdb.ny);

  fclose(f);

  if(n < 0){
    log_err(1, "Error writing info file.");
    return(1);
  }

  return(0);
}

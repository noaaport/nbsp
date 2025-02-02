/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "err.h"
#include "dcgini.h"

static int dcgini_csv_write_data(FILE *fp, struct dcgini_point_map_st *pm);

int dcgini_csv_write(char *file, struct dcgini_point_map_st *pm){

  FILE *fp;
  int status;

  if(strcmp(file, "-") == 0)
    fp = stdout;
  else{
    fp = fopen(file, "w");
    if(fp == NULL)
      log_err(1, "Cannot open %s", file);
  }

  status = dcgini_csv_write_data(fp, pm);

  if(fp != stdout)
    fclose(fp);

  if(status != 0)
    log_err(1, "Cannot write to %s", file);

  return(0);
}

static int dcgini_csv_write_data(FILE *fp, struct dcgini_point_map_st *pm){
  /*
   * Output the points's data in csv format.
   */
  struct dcgini_point_st *point = pm->points;
  size_t i;

  for(i = 0; i < pm->numpoints; ++i){
    if(fprintf(fp, "%.3f,%.3f,%d\n", point->lon, point->lat, point->level) < 0)
      return(-1);
    
    ++point;
  }

  return(0);
}

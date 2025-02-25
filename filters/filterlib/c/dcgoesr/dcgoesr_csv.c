/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 * Copyright (c) 2025 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
/*
 * The code is a copy of the dcgini_shp.c
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>	/* isnan */
#include "err.h"
#include "dcgoesr.h"

static int dcgoesr_csv_write_data(FILE *fp, struct dcgoesr_point_map_st *pm);

static int dcgoesr_csv_write_data(FILE *fp, struct dcgoesr_point_map_st *pm){
  /*
   * Output the points's data in csv format.
   */
  struct dcgoesr_point_st *point = pm->points;
  size_t i;

  /* exclude points that point to space */
  for(i = 0; i < pm->numpoints; ++i){
    if((isnan(point->lon) == 0) && (isnan(point->lat) == 0))
      if(fprintf(fp, "%f,%f,%d\n", point->lon, point->lat, point->level) < 0)
	return(-1);

    ++point;
  }

  return(0);
}

/*
 * Public - declared in dcgoesr.h
 */
int dcgoesr_csv_write(char *file, struct dcgoesr_point_map_st *pm){

  FILE *fp;
  int status;

  if(strcmp(file, "-") == 0)
    fp = stdout;
  else{
    fp = fopen(file, "w");
    if(fp == NULL)
      log_err(1, "Cannot open %s", file);
  }

  status = dcgoesr_csv_write_data(fp, pm);

  if(fp != stdout)
    fclose(fp);

  if(status != 0)
    log_err(1, "Cannot write to %s", file);

  return(0);
}

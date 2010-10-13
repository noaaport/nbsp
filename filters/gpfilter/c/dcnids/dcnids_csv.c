/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdio.h>
#include "dcnids.h"

int dcnids_csv_write(FILE *fp, struct dcnids_polygon_map_st *pm){
  /*
   * Output the polygon's data in csv format.
   */
  struct dcnids_polygon_st *polygon = pm->polygons;
  int i, j;

  for(i = 0; i < pm->numpolygons; ++i){
    for(j = 0; j < 4; ++j){
      if(fprintf(fp, "%.3f %.3f ", polygon->lon[j], polygon->lat[j]) < 0)
	return(-1);
    }
    if(fprintf(fp, "%.3f %.3f,", polygon->lon[0], polygon->lat[0]) < 0)
      return(-1);
    
    /*
     * Output the "code" (0-15) and the corresponding "level" value.
     */
    if(fprintf(fp, "%d,%d\n", polygon->code, polygon->level) < 0)
      return(-1);

    ++polygon;
  }

  return(0);
}

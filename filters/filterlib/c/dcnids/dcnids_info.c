/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdio.h>
#include "err.h"
#include "dcnids_info.h"

int dcnids_info_write(char *infofile, struct nids_data_st *nd){

  struct dcnids_polygon_map_st *pm = &(nd->polygon_map);
  FILE *f;
  int n;

  f = fopen(infofile, "w");
  if(f == NULL){
    log_err(1, "Cannot open info file.");
    return(1);
  }

  n = fprintf(f, "radseconds: %" PRIuMAX "\n",
	      (uintmax_t)nd->nids_header.unixseconds);
  if(n > 0)
    n = fprintf(f, "radmode: %d\n", nd->nids_header.pdb_mode);

  if(n > 0)
    n = fprintf(f, "prodcode: %d\n", nd->nids_header.pdb_code);

  if(n > 0)
    n = fprintf(f, "packetcode: %d\n", nd->radial_packet_header.packet_code);

  if(n > 0)
    n = fprintf(f, "lon: %.3f\n", nd->nids_header.lon);

  if(n > 0)
    n = fprintf(f, "lat: %.3f\n", nd->nids_header.lat);

  if(n > 0)
    n = fprintf(f, "height: %d\n", nd->nids_header.pdb_height);

  if(n > 0)
    n = fprintf(f, "bb: %f %f %f %f\n", pm->lon_min, pm->lat_min,
		pm->lon_max, pm->lat_max);

  fclose(f);

  if(n < 0){
    log_err(1, "Error writing info file.");
    return(1);
  }

  return(0);
}

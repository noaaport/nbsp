/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCGOESR_H
#define DCGOESR_H

#include <inttypes.h>	/* uint8_t */
#include <stddef.h>	/* size_t */

/* The projection-transformed data to lon/lat */
struct dcgoesr_point_st {
  double lon;	/* in degrees */
  double lat;	/* in degrees */
  uint8_t level; /* a copy of the cmi normalized to 0-255 */
};

struct dcgoesr_point_map_st {
  struct dcgoesr_point_st *points;
  size_t numpoints;
  /*
   * The "maximum enclosing rectangle" (bounding box)
   */ 
  double lon_min;
  double lat_min;
  double lon_max;
  double lat_max;
};

/* dcgoesr_shp.c */
int dcgoesr_shp_write(char *shpfile, char *shxfile,
		      struct dcgoesr_point_map_st *pm);

#endif

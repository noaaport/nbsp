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

/* Defs of structures used by the various modules */

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
  double lon1;
  double lat1;
  double lon2;
  double lat2;
};

#endif

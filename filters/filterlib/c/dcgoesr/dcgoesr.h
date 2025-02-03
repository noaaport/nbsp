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
   * "bounding box" (smallest rectangle that encloses the raw data)
   */
  double lon_min;
  double lat_min;
  double lon_max;
  double lat_max;
  /*
   * "maximum enclosing rectangle" (largest rectangle excluding background
   * points (level 0) in the determination of the limits).
   */ 
  double lon_ll;
  double lat_ll;
  double lon_ur;
  double lat_ur;
  /*
   * Some of the parameters extracted from the nc file are copied here
   * so that public functions only need the pmap structure (not the nc data st)
   */
  int nx;
  int ny;
};

/* dcgoesr_shp.c */
int dcgoesr_shp_write(char *shpfile, char *shxfile,
		      struct dcgoesr_point_map_st *pm);

/* dcgoesr_dbf.c */
int dcgoesr_dbf_write(char *file, struct dcgoesr_point_map_st *pm);

/* dcgoesr_info.c */
int dcgoesr_info_write(char *file, struct dcgoesr_point_map_st *pm);

/* dcgoesr_csv.c */
int dcgoesr_csv_write(char *file, struct dcgoesr_point_map_st *pm);

#endif

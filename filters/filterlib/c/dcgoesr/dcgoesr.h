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

/* convenience constants */
#define RAD_PER_DEG     0.017453		/* pi/180 */
#define DEG_PER_RAD	57.295780		/* 180/pi */

/* The projection-transformed data to lon/lat */
struct dcgoesr_point_st {
  float lon;	/* in degrees */
  float lat;	/* in degrees */
  uint8_t level; /* a copy of the cmi normalized to 0-255 */
};

struct dcgoesr_point_map_st {
  struct dcgoesr_point_st *points;
  size_t numpoints;
  /*
   * "bounding box" (smallest rectangle that encloses the raw data)
   */
  float lon_min;
  float lat_min;
  float lon_max;
  float lat_max;
  /*
   * "maximum enclosing rectangle" (largest rectangle excluding background
   * points (level 0) in the determination of the limits).
   */ 
  float lon_ll;
  float lat_ll;
  float lon_ur;
  float lat_ur;
  /*
   * Some of the parameters extracted from the nc file are stored/copied here
   * so that public functions only need the pmap structure (not the nc data st)
   * There are used, e.g., in the regrid asc function(s).
   */
  int nx;
  int ny;
  float lorigin; /* for the x,y -> lon,lat conversion */
  float x_min;
  float y_min;
  float x_max;
  float y_max;
  float dx;
  float dy;
};

/* The regridded data */

struct dcgoesr_grid_map_st {
  int *level;		/* declared int so that NODATA can be used */
  size_t numpoints;	/* nlon * nlat */
  size_t nlon;
  size_t nlat;
  float lon_min;
  float lat_min;
  float lon_max;
  float lat_max;
  float dlon;
  float dlat;
  float cellsize;	/* for square "asc" cell grids */
};

/* public functions */

/* dcgoesr_shp.c */
int dcgoesr_shp_write(char *shpfile, char *shxfile,
		      struct dcgoesr_point_map_st *pm);

/* dcgoesr_dbf.c */
int dcgoesr_dbf_write(char *file, struct dcgoesr_point_map_st *pm);

/* dcgoesr_info.c */
int dcgoesr_info_write(char *file, struct dcgoesr_point_map_st *pm);

/* dcgoesr_csv.c */
int dcgoesr_csv_write(char *file, struct dcgoesr_point_map_st *pm);

/* dcgoesr_asc.c */
int dcgoesr_asc_write(char *file, struct dcgoesr_grid_map_st *gm);

#endif

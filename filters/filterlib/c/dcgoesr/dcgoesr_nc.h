/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

#ifndef DCGOESRNC_H
#define DCGOESRNC_H

#include <inttypes.h>	/* uint8_t */
#include <stddef.h>	/* size_t */

/* The projection-transformed data to lon/lat */
struct dcgoesr_point_st {
  double lon;	/* in degrees */
  double lat;	/* in degrees */
  uint8_t level; /* cmi normalized to 0-255 */
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

/*
 * The data extracted from the nc file.
 *
 * All the data is stored in "data". x,y,cmi are pointers to the proper
 * place in data. For each x,y pair there is a lon, lat that must
 *  be calculated for each pair. They are stored in the point_map st.
 *
 * data_size is the total size of the data:
 *
 *   sizeof(double)*(nx + ny + Npoints)    (Npoints = nx * ny)
 *
 * x(i),y(j),cmi(i,j) (doubles)
 *
 * x,y are given in the file in radians while the
 * global tile_center_{lon,lat} are in degrees.
 *
 * All our calculated lon,lat variables are in degrees.
 */
struct goesr_st {
  int nx;	/* size of x */
  int ny;	/* size of y */
  int Npoints;	/* nx*ny */
  size_t data_size; /* total size of the data */
  double lorigin; /* for the x,y -> lon,lat conversion */
  double *data;	/* data storage */
  double *x;	/* x[i] - radians */
  double *y;	/* y[j] - radians */
  double *cmi;	/* size = nx*ny - "cmi(j,i)"  = cmi[k] with k = j*nx + i */
  /* global "attributes - info */
  double tclon;	/* tile center longitude - not in all files (e.g., tirs) */
  double tclat; /* tile center latitude - not in all files (e.g., tirs) */
  /* transformed data */
  struct dcgoesr_point_map_st pmap;
};

/* public functions */
void goesr_config(int c);	/* choose noaaport type or OR type file */
int goesr_create(int ncid, struct goesr_st **goesr);
void goesr_free(struct goesr_st *goesr);

#endif

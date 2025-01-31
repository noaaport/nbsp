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
#include "dcgoesr.h"	/* definitions of the pointmap st */

/*
 * The x,y,cmi data is extracted from the nc file.
 * From the cmi we determine the "level" which is the cmi normalized to 0-255.
 *
 * All the data is stored in "data". x,y,cmi,level are pointers to the proper
 * place in data. For each x,y pair there is a lon, lat that must
 * be calculated for each pair. They are stored in the point_map st.
 *
 *
 * data_size is the total size of the data:
 *
 *   sizeof(double)*(nx + ny + Npoints) + sizeof(uint8_t)*Npoints
 *
 * where
 *
 *   Npoints = nx*ny.
 *
 * x(i),y(j),cmi(i,j) (doubles)
 * level(i,j) (uint8_t)
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
  uint8_t *level; /* cmi normalized to 0-255 - also stored in "data" */
  struct dcgoesr_point_map_st pmap;	/* contains a copy of the "levels" */
};

/* public functions */
void goesr_config(int c);	/* choose noaaport type or OR type file */
int goesr_create(int ncid, struct goesr_st **goesr);
void goesr_free(struct goesr_st *goesr);

#endif

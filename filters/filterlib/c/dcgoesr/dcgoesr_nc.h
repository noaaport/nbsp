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

/*
 * All the data is stored in "data". x,y,cmi,lon,lat,level are pointers to data.
 * For each x,y pair there is a lon, lat that must be calculated for each
 * pair.
 *
 * data_size is the total size of the data:
 *
 *   sizeof(double)*(nx + ny + 3*Npoints) + sizeof(uint8_t)*Npoints
 *
 * x(i),y(j),(cmi,lon,lat)(i,j) (doubles)
 * level(i,j) (unit8_t)
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
  double *lon;	/* for each cmi[k] as above, there is a lon[k] */
  double *lat;	/* for each cmi[k] as above, there is a lat[k] */
  uint8_t *level; /* normalized cmi to 0-255 */
  /* global "attributes - info */
  double tclon;	/* tile center longitude */
  double tclat; /* tile center latitude */
  double lon1;	/* lower left lon */
  double lat1;	/* lower left lat */
  double lon2;	/* upper right lon */
  double lat2;	/* upper right lat */
};

/* public functions */
void goesr_config(int c);	/* choose noaaport type or OR type file */
int goesr_create(int ncid, struct goesr_st **goesr);
void goesr_free(struct goesr_st *goesr);

#endif

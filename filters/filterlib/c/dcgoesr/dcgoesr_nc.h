/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

#ifndef DCGOESRNC_H
#define DCGOESRNC_H

/*
 * All the data is stored in "data". x,y,cmi, lon, lat are pointers to data.
 * For each x,y pair there is a lon, lat that must be calculated for each
 * pair.
 */
struct goesr_st {
  int nx;	/* size of x */
  int ny;	/* size of y */
  int ndata;	/* nx + ny + 3*nx*ny */
  double lorigin; /* for the x,y -> lon,lat conversion */
  double *data;	/* data storage */
  double *x;	/* x[i] - radians */
  double *y;	/* y[j] - radians */
  double *cmi;	/* size = nx*ny - "cmi(j,i)"  = cmi[k] with k = j*nx + i */
  double *lon;	/* for each cmi[k] as above, there is a lon[k] */
  double *lat;	/* for each cmi[k] as above, there is a lat[k] */
  /* global "attributes - info */
  double tclon;	/* tile center longitude */
  double tclat; /* tile center latitude */
  double lon1;	/* lower left lon */
  double lat1;	/* lower left lat */
  double lon2;	/* upper right lon */
  double lat2;	/* upper right lat */
};

/* public functions */
int goesr_create(int ncid, struct goesr_st **goesr);
void goesr_free(struct goesr_st *goesr);

#endif

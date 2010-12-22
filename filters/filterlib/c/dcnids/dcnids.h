/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <inttypes.h>
#include <stdio.h>

#ifndef DCNIDS_H
#define DCNIDS_H

struct dcnids_polygon_st {
  double lon[4];
  double lat[4];
  int code;	/* the level code value for this polygon */
  int level;	/* the level value */
};

struct dcnids_polygon_map_st {
  struct dcnids_polygon_st *polygons;
  int numpolygons;
  double lon_min;
  double lat_min;
  double lon_max;
  double lat_max;
  int level_min;	/* These can be set to filter out data */
  int level_max;
  int flag_usefilter;
};

struct dcnids_shp_st {
  unsigned char *b;	/* buffer */
  uint32_t size;
  uint32_t shpsize;
  uint32_t shxsize;
};

/* transform */
void dcnids_sine_cosine(double theta_deg,
			double *sin_theta, double *cos_theta);

void dcnids_define_polygon(double lon0, double lat0,
			   double r1, double r2,
			   double sin_theta1, double cos_theta1,
			   double sin_theta2, double cos_theta2,
			   struct dcnids_polygon_st *p);

void dcnids_xytolatlon(double lon0, double lat0,
		       double x, double y,
		       double *lon, double *lat);

void dcnids_polygonmap_bb(struct dcnids_polygon_map_st *pm);

/* shp */
int dcnids_shp_write(char *shpfile, char *shxfile,
		     struct dcnids_polygon_map_st *pm);

/* csv */
int dcnids_csv_write(FILE *fp, struct dcnids_polygon_map_st *pm);

/* dbf */
int dcnids_dbf_write(char *dbfname, struct dcnids_polygon_map_st *pm);

/* bunz */
int dcnids_bunz(unsigned char **data, unsigned int *datasize, int *bzerror);

#if 0
int dcnids_dbf_write(char *dbfname,
		     char *codename,
		     char *levelname,
		     struct dcnids_polygon_map_st *pm);
#endif

#endif

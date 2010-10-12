/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <math.h>
#include "dcnids.h"

#define KM_PER_DEG	111.111		/* R_E * pi/180 */
#define RAD_PER_DEG	0.0174		/* pi/180 */

void dcnids_define_polygon(double lon0, double lat0,
			   double r1, double r2,
			   double theta1_deg, double theta2_deg,
			   struct dcnids_polygon_st *p){
  /*
   * theta1_deg and theta2_deg must be in degrees.
   * r1, r2 in km.
   */
  double x, y;
  double theta1, theta2;
  
  theta1 = theta1_deg * RAD_PER_DEG;
  theta2 = theta2_deg * RAD_PER_DEG;

  /* ll */
  x = r1 * sin(theta1);
  y = r1 * cos(theta1);
  dcnids_xytolatlon(lon0, lat0, x, y, &p->lon[0], &p->lat[0]);

  /* ul */
  x = r2 * sin(theta1);
  y = r2 * cos(theta1);
  dcnids_xytolatlon(lon0, lat0, x, y, &p->lon[1], &p->lat[1]);  

  /* ur */
  x = r2 * sin(theta2);
  y = r2 * cos(theta2);
  dcnids_xytolatlon(lon0, lat0, x, y, &p->lon[2], &p->lat[2]);  

  /* lr */
  x = r1 * sin(theta2);
  y = r1 * cos(theta2);
  dcnids_xytolatlon(lon0, lat0, x, y, &p->lon[3], &p->lat[3]);
}
  
void dcnids_xytolatlon(double lon0, double lat0,
		       double x, double y,
		       double *lon, double *lat){
  /*
   * x, y are the coordinates of a point (in km) relative to the
   * reference lon0 and lat0 (the radar site).
   */
  double dlat, dlon, avg_lat;  

  dlat = y/KM_PER_DEG;
  *lat = lat0 + dlat;
  avg_lat = (lat0 + dlat/2.0)*RAD_PER_DEG;
  dlon = x/(KM_PER_DEG*cos(avg_lat));
  *lon = lon0 + dlon;
}

void dcnids_polygonmap_bb(struct dcnids_polygon_map_st *pm){

  int i, j;

  pm->lon_min = 180.0;
  pm->lon_max = -180.0;
  pm->lat_min = 180.0;
  pm->lon_max = -180.0;
  
  for(j = 0; j < pm->numpolygons; ++j){
    for(i = 0; i < 4; ++i){
      if(pm->polygons[j].lon[i] < pm->lon_min)
	pm->lon_min = pm->polygons[j].lon[i];

      if(pm->polygons[j].lon[i] > pm->lon_max)
	pm->lon_max = pm->polygons[j].lon[i];

      if(pm->polygons[j].lat[i] < pm->lat_min)
	pm->lat_min = pm->polygons[j].lat[i];

      if(pm->polygons[j].lat[i] > pm->lat_max)
	pm->lat_max = pm->polygons[j].lat[i];
    }
  }
}

#if 0
void ijtolatlon(double lat0, double lon0,
		int i, int j,
		double *lat, double *lon){
  /*
   * The arguments i,j are the int16 extracted from the packet; they
   * are the x,y coordinates, in units of km/4 (one quarter of km),
   * relative to the radar lat0, lon0.
   * The constant KM_PER_DEG is the length of a 1 degree arc in the surface
   * of the EARTH:
   */
  double x, y, dlat, dlon, avg_lat_rad;

  /* The coordinates in km */
  x = (double)i/4.0;
  y = (double)j/4.0;

  dlat = y/KM_PER_DEG;
  *lat = lat0 + dlat;

  avg_lat_rad = (lat0 + dlat/2.0)*RAD_PER_DEG;
  dlon = x/(KM_PER_DEG*cos(avg_lat_rad));
  *lon = lat0 + dlon;
}
#endif

/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
/* #include <stdio.h>	XXX - only for debugging */
#include <math.h>
#include <stdlib.h>
#include "err.h"
#include "dcgini_const.h"
#include "dcgini_transform.h"
#include "dcgini_transform_priv.h"

static void dcgini_pointmap_bb(struct dcgini_point_map_st *pm);

int dcgini_transform_data(struct dcgini_st *dcg){

  struct nesdis_proj_str_st pstr;
  struct nesdis_proj_llc_st pllc;
  struct nesdis_proj_mer_st pmer;
  struct dcgini_point_st *points;
  size_t numpoints;
  unsigned char *datap;
  int i, j;
  double lon_deg, lat_deg;

  /*
   * Call nesdis_proj_xxx_init() to initalize the pxxx, and then call
   * nesdis_proj_xxx_transform() repeatedly to transform each point.
   */
  if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_STR)
    nesdis_proj_str_init(&dcg->pdb, &pstr);
  else if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_LLC)
    nesdis_proj_llc_init(&dcg->pdb, &pllc);
  else if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_MER)
    nesdis_proj_mer_init(&dcg->pdb, &pmer);
  else{
    log_warnx("Unsupported map projection: %d", dcg->pdb.map_projection);
    return(1);
  }

  /* Allocate storage for the pointmap */
  numpoints = dcg->pdb.nx * dcg->pdb.ny;

  if(dcg->ginidata.data_size < numpoints){
    log_warnx("The data size is smaller than nx * ny");
    return(1);
  }

  points = calloc(numpoints, sizeof(struct dcgini_point_st));
  if(points == NULL){
    log_err(1, "Cannot allocate memory for pointmap");
    return(-1);
  }

  dcg->pointmap.numpoints = numpoints;
  dcg->pointmap.points = points;

  datap = dcg->ginidata.data;

  /*
   * Assume the scan is top to bottom, left to right; in principle
   * we should check the pdb.scan_mode flag.
   */
  for(j = dcg->pdb.ny - 1; j >= 0; --j){
    for(i = 0; i < dcg->pdb.nx; ++i){
      if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_STR)
	nesdis_proj_str_transform(&pstr, i, j, &lon_deg, &lat_deg);
      else if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_LLC)
	nesdis_proj_llc_transform(&pllc, i, j, &lon_deg, &lat_deg);
      else if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_MER)
	nesdis_proj_mer_transform(&pmer, i, j, &lon_deg, &lat_deg);

      /* XXX fprintf(stdout, "%f %f\n", lon_deg, lat_deg); */
      points->lon = lon_deg;
      points->lat = lat_deg;
      points->level = (int)*datap;
      ++points;
      ++datap;
    }
    /* XXX exit(0); */
  }

  dcgini_pointmap_bb(&dcg->pointmap);

  return(0);
}

static void dcgini_pointmap_bb(struct dcgini_point_map_st *pm){

  size_t i;

  pm->lon_min = 180.0;
  pm->lon_max = -180.0;
  pm->lat_min = 180.0;
  pm->lon_max = -180.0;
  
  for(i = 0; i < pm->numpoints; ++i){
    if(pm->points[i].lon < pm->lon_min)
      pm->lon_min = pm->points[i].lon;

    if(pm->points[i].lon > pm->lon_max)
      pm->lon_max = pm->points[i].lon;

    if(pm->points[i].lat < pm->lat_min)
      pm->lat_min = pm->points[i].lat;

    if(pm->points[i].lat > pm->lat_max)
      pm->lat_max = pm->points[i].lat;
  }
}

void nesdis_proj_str_init(struct nesdis_pdb_st *npdb,
			  struct nesdis_proj_str_st *pstr){

  double alpha;
  double r;

  pstr->s = 1.0;
  if(npdb->proj_center_flag != 0){
    pstr->s = -1.0;
  }
  pstr->lov_rad = npdb->lov_rad;

  alpha = 1.0/sin(M_PI/3.0);
  r = RE_METERS * tan(M_PI_4 - 0.5 * pstr->s * npdb->lat1_rad);

  pstr->x1 = r * sin(npdb->lon1_rad - npdb->lov_rad);
  pstr->y1 = -pstr->s * r * cos(npdb->lon1_rad - npdb->lov_rad);
  pstr->dx = alpha * npdb->dx_meters;
  pstr->dy = alpha * npdb->dy_meters;
}

void nesdis_proj_str_transform(struct nesdis_proj_str_st *pstr,
			       int i,
			       int j,
			       double *lon_deg,
			       double *lat_deg){
  double x, y;
  double lon_rad, lat_rad;
  double r;

  x = pstr->x1 + (double)i * pstr->dx;
  y = pstr->y1 + pstr->s * (double)j * pstr->dy;
  r = sqrt(pow(x, 2.0) + pow(y, 2.0));
 
  lon_rad = pstr->lov_rad + atan2(x, -pstr->s * y);
  lat_rad = pstr->s * (M_PI_2 - atan2(r, RE_METERS));
  
  *lon_deg = lon_rad * DEG_PER_RAD;
  *lat_deg = lat_rad * DEG_PER_RAD;
}

void nesdis_proj_llc_init(struct nesdis_pdb_st *npdb,
			  struct nesdis_proj_llc_st *pllc){
  double alpha;
  double psi;
  double a, b;

  pllc->s = 1.0;
  if(npdb->proj_center_flag != 0){
    pllc->s = -1.0;
  }
  pllc->lov_rad = npdb->lov_rad;

  psi = M_PI_2 - fabs(npdb->latin_rad);
  pllc->cos_psi = cos(psi);
  pllc->r_E = RE_METERS/pllc->cos_psi;

  alpha = pow(tan(psi/2.0), pllc->cos_psi) / sin(psi);
  a = pow(tan(M_PI_4 - 0.5 * pllc->s * npdb->lat1_rad), pllc->cos_psi);
  b = pllc->cos_psi * (npdb->lon1_rad - npdb->lov_rad);

  pllc->x1 = pllc->r_E * a * sin(b);
  pllc->y1 = -pllc->s * pllc->r_E * a * cos(b);
  pllc->dx = alpha * npdb->dx_meters;
  pllc->dy = alpha * npdb->dy_meters;

  /*
  fprintf(stdout, "%f %f %f %f %f\n", pllc->x1, pllc->y1, pllc->dx, pllc->dy,
  pllc->s);
  exit(0);
  */
}

void nesdis_proj_llc_transform(struct nesdis_proj_llc_st *pllc,
			       int i,
			       int j,
			       double *lon_deg,
			       double *lat_deg){
  double x, y;
  double lon_rad, lat_rad;
  double rr;
  double a;
  
  x = pllc->x1 + (double)i * pllc->dx;
  y = pllc->y1 + pllc->s * (double)j * pllc->dy;

  rr = sqrt(pow(x, 2.0) + pow(y, 2.0)) / pllc->r_E;
  a = 1.0/pllc->cos_psi;

  lon_rad = pllc->lov_rad + atan2(x, -pllc->s * y) / pllc->cos_psi;
  lat_rad = pllc->s * (M_PI_2 - 2.0 * atan(pow(rr, a)));
  
  *lon_deg = lon_rad * DEG_PER_RAD;
  *lat_deg = lat_rad * DEG_PER_RAD;

  /* XXX fprintf(stdout, "%f %f\n", x, *lon_deg); */
}

void nesdis_proj_mer_init(struct nesdis_pdb_st *npdb,
			  struct nesdis_proj_mer_st *pmer){

  double r1, r2;
  double b, d;
  double n;

  /* center longitude ? */
  pmer->lon0_rad = 0.5 * (npdb->lon1_rad - npdb->lon2_rad);

  d = npdb->lon1_rad - pmer->lon0_rad;
  r1 = tan(npdb->lat1_rad);
  r2 = cos(d);
  b = cos(npdb->lat1_rad) * sin(d);  
  pmer->x1 = 0.5 * log((1.0 + b)/(1.0 - b));
  pmer->y1 = atan2(r1, r2);

  d = npdb->lon2_rad - pmer->lon0_rad;
  r1 = tan(npdb->lat2_rad);
  r2 = cos(d);
  b = cos(npdb->lat2_rad) * sin(d);
  pmer->x2 = 0.5 * log((1.0 + b)/(1.0 - b));
  pmer->y2 = atan2(r1, r2);

  n = (double)npdb->nx - 1.0;
  pmer->dx = (pmer->x2 - pmer->x1)/n;
  n = (double)npdb->ny - 1.0;
  pmer->dy = (pmer->y2 - pmer->y1)/n;
}

void nesdis_proj_mer_transform(struct nesdis_proj_mer_st *pmer,
			       int i,
			       int j,
			       double *lon_deg,
			       double *lat_deg){
  double x, y;
  double lon_rad, lat_rad;

  x = pmer->x1 + i * pmer->dx;
  y = pmer->y1 + j * pmer->dy;

  lon_rad = pmer->lon0_rad + atan2(sinh(x), cos(y));
  lat_rad = asin(sin(y)/cosh(x));

  *lon_deg = lon_rad * DEG_PER_RAD;
  *lat_deg = lat_rad * DEG_PER_RAD;
}

/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <math.h>
#include "dcgini_const.h"
#include "dcgini_transform.h"

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
}

void nesdis_proj_llc_transform(struct nesdis_proj_llc_st *pllc,
			       int i,
			       int j,
			       double *lon_deg,
			       double *lat_deg){
  double x, y;
  double lon_rad, lat_rad;
  double rr;
  double a, b;
  
  x = pllc->x1 + (double)i * pllc->dx;
  y = pllc->y1 + pllc->s * (double)j * pllc->dy;

  rr = sqrt(pow(x, 2.0) + pow(y, 2.0)) / pllc->r_E;
  a = 1.0/pllc->cos_psi;

  lon_rad = pllc->lov_rad * atan2(x, -pllc->s * y) / pllc->cos_psi;
  lat_rad = pllc->s * (M_PI_2 - 2.0 * atan(pow(rr, a)));
  
  *lon_deg = lon_rad * DEG_PER_RAD;
  *lat_deg = lat_rad * DEG_PER_RAD;
}

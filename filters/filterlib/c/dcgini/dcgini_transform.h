/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCGINI_TRANSFORM_H
#define DCGINI_TRANSFORM_H

#include "dcgini_pdb.h"

/* polar str */
struct nesdis_proj_str_st {
  double s;
  double lov_rad;	/* this is a copy from the pdb */
  double x1;
  double y1;
  double dx;
  double dy;
};

/* llc */
struct nesdis_proj_llc_st {
  double s;
  double lov_rad;	/* this is a copy from the pdb */
  double cos_psi;
  double r_E;
  double x1;
  double y1;
  double dx;
  double dy;
};

/* mercator */
struct nesdis_proj_mer_st {
  double lon0_rad;	/* center longitude */
  double x1;
  double y1;
  double x2;
  double y2;
  double dx;
  double dy;
};

void nesdis_proj_str_init(struct nesdis_pdb_st *npdb,
			  struct nesdis_proj_str_st *pstr);
void nesdis_proj_str_transform(struct nesdis_proj_str_st *pstr,
			       int i,
			       int j,
			       double *lon_deg,
			       double *lat_deg);

void nesdis_proj_llc_init(struct nesdis_pdb_st *npdb,
			  struct nesdis_proj_llc_st *pllc);
void nesdis_proj_llc_transform(struct nesdis_proj_llc_st *pllc,
			       int i,
			       int j,
			       double *lon_deg,
			       double *lat_deg);

void nesdis_proj_mer_init(struct nesdis_pdb_st *npdb,
			  struct nesdis_proj_mer_st *pmer);
void nesdis_proj_mer_transform(struct nesdis_proj_mer_st *pmer,
			       int i,
			       int j,
			       double *lon_deg,
			       double *lat_deg);

#endif

/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCGINI_TRANSFORM_PRIV_H
#define DCGINI_TRANSFORM_PRIV_H

#include "dcgini_pdb.h"
#include "dcgini_projections.h"

void nesdis_proj_str_init(struct nesdis_pdb_st *npdb,
			  struct nesdis_proj_str_st *pstr);
void nesdis_proj_str_ij_lonlat(struct nesdis_proj_str_st *pstr,
			       int i,
			       int j,
			       double *lon_deg,
			       double *lat_deg);
void nesdis_proj_str_lonlat_ij(struct nesdis_proj_str_st *pstr,
			       double lon_deg,
			       double lat_deg,
			       double *ii,
			       double *jj);

void nesdis_proj_llc_init(struct nesdis_pdb_st *npdb,
			  struct nesdis_proj_llc_st *pllc);
void nesdis_proj_llc_ij_lonlat(struct nesdis_proj_llc_st *pllc,
			       int i,
			       int j,
			       double *lon_deg,
			       double *lat_deg);
void nesdis_proj_llc_lonlat_ij(struct nesdis_proj_llc_st *pllc,
			       double lon_deg,
			       double lat_deg,
			       double *ii,
			       double *jj);

void nesdis_proj_mer_init(struct nesdis_pdb_st *npdb,
			  struct nesdis_proj_mer_st *pmer);
void nesdis_proj_mer_ij_lonlat(struct nesdis_proj_mer_st *pmer,
			       int i,
			       int j,
			       double *lon_deg,
			       double *lat_deg);
void nesdis_proj_mer_lonlat_ij(struct nesdis_proj_mer_st *pmer,
			       double lon_deg,
			       double lat_deg,
			       double *ii,
			       double *jj);
#endif

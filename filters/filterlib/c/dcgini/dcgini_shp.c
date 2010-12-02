/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdlib.h>
#include "err.h"
#include "dcgini_pdb.h"
#include "dcgini_shp.h"
#include "dcgini_transform.h"

int dcgini_create_pointmap(struct dcgini_st *dcg){

  struct nesdis_proj_str_st pstr;
  struct nesdis_proj_llc_st pllc;
  struct nesdis_proj_llc_st pmer;
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

  for(j = 0; j < dcg->pdb.ny; ++j){
    for(i = 0; i < dcg->pdb.nx; ++i){
      if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_STR)
	nesdis_proj_str_transform(&pstr, i, j, &lon_deg, &lat_deg);
      else if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_LLC)
	nesdis_proj_llc_transform(&pllc, i, j, &lon_deg, &lat_deg);
      else if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_MER)
	nesdis_proj_mer_transform(&pmer, i, j, &lon_deg, &lat_deg);

      points->lon = lon_deg;
      points->lat = lat_deg;
      points->level = (int)*datap;
      ++points;
      ++datap;
    }
  }

  return(0);
}

int dcgini_create_shp_data(struct dcgini_st *dcg){

  int status = 0;

  return(status);
}

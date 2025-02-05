/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 * Copyright (c) 2025 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "err.h"
#include "dcgoesr_regrid_asc.h"		/* includes dcgoesr.h */

/*
 * The assumption is that in the nc data, the indices
 * i,j of a point level(j,i) and the corresponding (lon,lat)
 * can be related by
 *
 *  lon = lon1 + dlon * i
 *  lat = lat1 + dlat * j
 *
 * where
 *
 *  dlon = (lon2 - lon1)/(nx - 1)
 *  dlat = (lat2 - lat1)/(ny - 1)
 *
 * Inverting,
 *
 *  i = (1/dlon) * (lon - lon1)
 *  j = (1/dlon) * (lat - lat1)
 *
 * The anzatz is to assume that this is how we can assign a level(j,i)
 * to any (lon,lat) (i.e., a linear fit).
 */

int dcgoesr_regrid_data_asc(struct dcgoesr_point_map_st *pmap,
			    char *llur_str,
			    int f_llur_str_diff,
			    struct dcgoesr_point_map_st *gmap) {
  /*
   * The distinctive feature in this function is that here we define
   * the cellsize (dlon, dlat) such that it is a square cell, as 
   * required by the "asc" format.
   *
   * This function assumes that the bounding box and the
   * maximum enclosing rectangle have been determined.
   */
  size_t numpoints;
  int *datap;
  size_t i, j, k, l;
  double ii, jj, lon_deg, lat_deg;
  double cellsize;	/* asc format requires square cell size */
  double dlon, dlat;
  double rlon1, rlat1, rlon2, rlat2;

  dlon = (pmap->lon_max - pmap->lon_min)/(pmap->nx - 1);
  dlat = (pmap->lat_max - pmap->lat_min)/(pmap->ny - 1);
  if(dlat < dlon)
    cellsize = dlat;
  else
    cellsize = dlon;

  gmap->dlon = cellsize;
  gmap->dlat = cellsize;
  gmap->cellsize = cellsize;

  /* the default limits */
  gmap->lon1 = pmap->lon_ll;
  gmap->lat1 = pmap->lat_ll;
  gmap->lon2 = pmap->lon_ur;
  gmap->lat2 = pmap->lat_ur;

  /* optional limits */
  if(llur_str != NULL){
    if(sscanf(llur_str, "%lf,%lf,%lf,%lf",
	      &rlon1, &rlat1, &rlon2, &rlat2) != 4){
      log_errx(0, "Invalid value of enclosing rectangle limits");
      return(1);
    }

    if(f_llur_str_diff != 0){
      /* shrink the rectangle by the specified amouns */
      gmap->lon1 += rlon1;
      gmap->lat1 += rlat1;
      gmap->lon2 -= rlon2;
      gmap->lat2 -= rlat2;
    }else{
      gmap->lon1 = rlon1;
      gmap->lat1 = rlat1;
      gmap->lon2 = rlon2;
      gmap->lat2 = rlat2;
    }
  }

  /*
   * Since we ((may) have redefined the limits, and redefined the cellsize
   * we have to redetermine the number of points.
   */
  gmap->nlon = (gmap->lon2 - gmap->lon1)/cellsize + 1.0;
  gmap->nlat = (gmap->lat2 - gmap->lat1)/cellsize + 1.0;

  /* XXX
  fprintf(stdout, "%f %f", dcg->gridmap.lat2_deg, dcg->gridmap.lat1_deg);
  fprintf(stdout, "%u %u\n", dcg->gridmap.nlon, dcg->gridmap.nlat);
  */

  /* Allocate storage for the levels array */
  numpoints = gpam->nlon * gmapnlat;
  gmap->level = calloc(numpoints, sizeof(gmap->level));
  if(gmap->level == NULL){
    log_err(0, "Cannot allocate memory for the regrid map");
    return(-1);
  }

  if(DCGOESR_GRID_MAP_NODATA != 0){
    for(k = 0; k < numpoints; ++k){
      gmaplevel[k] = DCGOESR_GRID_MAP_NODATA;
    }
  }

  datap = gmap->level;
  gmap->numpoints = numpoints;

  /*
   * The strategy now is to loop through all the gm->level[k],
   * with k = j*nlon + i. For each i,j we can calculate the
   * the corresponding (lon,lat) of the square grid
   *
   *   gm_lon(k) = gm_lon1 + cellsize * i
   *   gm_lat(k) = gm_lat1 + cellsize * j
   *
   * Then for that (gm_lon(k),gm_lat(k)) we calculate the (ii,jj) in the
   * original grid
   *
   *  ii = (1/pm_dlon) * (gm_lon(k) - lon_min)
   *  jj = (1/pm_dlat) * (gm_lat(k) - lat_min)
   *
   * and set
   *
   * gm->level[kk] = pm->point[k].
   */
   
  /*
   * Store the values in the order defined by the ArcInfo ASCII Grid format:
   * the origin of the grid is the upper left and terminates at the lower right.
   * (http://docs.codehaus.org/display/GEOTOOLS/ArcInfo+ASCII+Grid+format)
   *
   * Since l is unsigned, we replace the for(...; l >= 0; ...) by
   * what appears below.
   */ 
  for(l = dcg->gridmap.nlat - 1; l < SIZE_T_MAX; --l){
    for(k = 0; k < dcg->gridmap.nlon; ++k){
      lon_deg = dcg->gridmap.lon1_deg + (double)k * dcg->gridmap.dlon_deg;
      lat_deg = dcg->gridmap.lat1_deg + (double)l * dcg->gridmap.dlat_deg;

      if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_STR)
	nesdis_proj_str_lonlat_ij(&dcg->pstr, lon_deg, lat_deg, &ii, &jj);
      else if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_LLC)
	nesdis_proj_llc_lonlat_ij(&dcg->pllc, lon_deg, lat_deg, &ii, &jj);
      else if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_MER)
	nesdis_proj_mer_lonlat_ij(&dcg->pmer, lon_deg, lat_deg, &ii, &jj);

      if((ii < 0.0) || (ii > (double)dcg->pdb.nx - 1))
	*datap = DCGINI_GRID_MAP_NODATA;
      else if((jj < 0.0) || (jj > (double)dcg->pdb.ny - 1))
	*datap = DCGINI_GRID_MAP_NODATA;
      else{
	i = LROUND_SIZE_T(ii);
	j = dcg->pdb.ny - 1 - LROUND_SIZE_T(jj);    /* from top to bottom */
	*datap = (int)dcg->ginidata.data[j * dcg->pdb.nx + i];
      }
      ++datap;
    }
  }

  return(0);
}

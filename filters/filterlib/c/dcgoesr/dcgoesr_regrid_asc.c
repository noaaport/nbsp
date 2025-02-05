/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 * Copyright (c) 2025 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdlib.h>
#include <stdio.h>	/* only for debuging */
#include <string.h>
#include "err.h"
#include "dcgoesr_regrid_asc.h"		/* includes dcgoesr.h */

/*
 * The convention in the pointmap is that
 *
 *   level[k] = level(j,i)   (k = nx * j + i)
 *
 * and the corresponding point is (lon[k],lat[k]).
 *
 * The assumption now is that the (lon,lat) can be related to (i,j) by
 *
 *  lon = lon_min + dlon * i
 *  lat = lat_max - dlat * j	(top to bottom)
 *
 * Then
 *
 *  dlon = (lon_max - lon_min)/(nx - 1)
 *  dlat = (lat_max - lat_min)/(ny - 1)
 *
 * and inverting,
 *
 *  i = (1/dlon) * (lon - lon_min)
 *  j = (1/dlat) * (lat_max - lat)
 *
 * The anzatz is to assume that this is how we can assign a level(j,i)
 * to any (lon,lat).
 */

/*
 * We could use lround(), but in linux it seems to be a pain
 * to include it (define -fno-bultiin, or -std=c99, etc). It is
 * assumed that the argument x has already been checked to be positive
 * and within bounds.
 */
#define LROUND_SIZE_T(x) (size_t)(x + 0.5)

int dcgoesr_regrid_data_asc(struct dcgoesr_point_map_st *pmap,
			    char *llur_str,
			    int f_llur_str_diff,
			    struct dcgoesr_grid_map_st *gmap) {
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
  size_t i, j, k;
  size_t x, y;
  double ii, jj, lon, lat;
  double cellsize;	/* asc format requires square cell size */
  double dlon, dlat;
  double rlon_min, rlat_min, rlon_max, rlat_max;

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
  gmap->lon_min = pmap->lon_ll;
  gmap->lat_min = pmap->lat_ll;
  gmap->lon_max = pmap->lon_ur;
  gmap->lat_max = pmap->lat_ur;

  /* optional limits */
  if(llur_str != NULL){
    if(sscanf(llur_str, "%lf,%lf,%lf,%lf",
	      &rlon_min, &rlat_min, &rlon_max, &rlat_max) != 4){
      log_errx(0, "Invalid value of enclosing rectangle limits");
      return(1);
    }

    if(f_llur_str_diff != 0){
      /* shrink the rectangle by the specified amouns */
      gmap->lon_min += rlon_min;
      gmap->lat_min += rlat_min;
      gmap->lon_max -= rlon_max;
      gmap->lat_max -= rlat_max;
    }else{
      gmap->lon_min = rlon_min;
      gmap->lat_min = rlat_min;
      gmap->lon_max = rlon_max;
      gmap->lat_max = rlat_max;
    }
  }

  /*
   * Since we ((may) have redefined the limits, and redefined the cellsize
   * we have to redetermine the number of points.
   */
  gmap->nlon = (gmap->lon_max - gmap->lon_min)/cellsize + 1.0;
  gmap->nlat = (gmap->lat_max - gmap->lat_min)/cellsize + 1.0;

  /*
  fprintf(stdout, "%f %f\n", gmap->lon_min, gmap->lat_min);
  fprintf(stdout, "%zu %zu\n", gmap->nlon, gmap->nlat);
  */
  
  /* Allocate storage for the levels array */
  numpoints = gmap->nlon * gmap->nlat;
  gmap->level = calloc(numpoints, sizeof(gmap->level));
  if(gmap->level == NULL){
    log_err(0, "Cannot allocate memory for the regrid map");
    return(-1);
  }

  if(DCGOESR_GRID_MAP_NODATA != 0){
    for(k = 0; k < numpoints; ++k){
      gmap->level[k] = DCGOESR_GRID_MAP_NODATA;
    }
  }

  datap = gmap->level;
  gmap->numpoints = numpoints;

  /*
   * The strategy now is to loop through all the gm->level[k],
   * with k = j*nlon + i. For each i,j we can calculate the
   * the corresponding (lon,lat) of the square grid
   *
   *   gm_lon(k) = gm_lon_min + cellsize * i
   *   gm_lat(k) = gm_lat_max - cellsize * j
   *
   * Then for that (gm_lon(k),gm_lat(k)) we calculate the (ii,jj) in the
   * original grid
   *
   *  ii = (1/pm_dlon) * (pm_lon(k) - pm_lon_min)
   *  jj = (1/pm_dlat) * (pm_lat_max - pm_lat(k))
   *
   * and set
   *
   * gm->level[kk] = pm->point.level[k]  (kk = jj* nlat + i)
   */
   
  for(j = 0; j < gmap->nlat; ++j){
    for(i = 0; i < gmap->nlon; ++i){
      lon = gmap->lon_min + cellsize * i;
      lat = gmap->lat_max - cellsize * j;
      
      ii = (1/dlon) * (lon - pmap->lon_min);
      jj = (1/dlat) * (pmap->lat_max - lat);

      if((ii < 0.0) || (ii > (double)pmap->nx - 1.0))
	*datap = DCGOESR_GRID_MAP_NODATA;
      else if((jj < 0.0) || (jj > (double)pmap->ny - 1))
	*datap = DCGOESR_GRID_MAP_NODATA; 
      else{
	x = LROUND_SIZE_T(ii);
	y = LROUND_SIZE_T(jj);
	*datap = (int)pmap->points[y * pmap->nx + x].level;
      }
      ++datap;
    }
  }

  return(0);
}

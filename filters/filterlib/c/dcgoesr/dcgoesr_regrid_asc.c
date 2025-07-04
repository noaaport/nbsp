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
#include "dcgoesr_xy2lonlat.h"		/* lonlat2xy */
#include "dcgoesr_regrid_asc.h"		/* includes dcgoesr.h */

/*
 * The convention in the pointmap is that
 *
 *   level[k] = level(j,i)   (k = nx * j + i)
 *
 * and the corresponding point is (lon[k],lat[k]).
 *
 * The assumption now is that the corresponding (x,y) [not the (lon,lat)!]
 * can be related to (i,j) by
 *
 *  x = x_min + dx * i
 *  y = y_max - dy * j	(top to bottom)
 *
 * Then
 *
 *  dx = (x_max - x_min)/(nx - 1)
 *  dy = (y_max - y_min)/(ny - 1)
 *
 * and inverting,
 *
 *  i = (1/dx) * (x - x_min)
 *  j = (1/dy) * (y_max - y)
 *
 * The anzatz is to assume that this is how we can assign a level(j,i)
 * to any (lon,lat): from the given (lon,lat) we determine the (x,y)
 * by the function "lonlat2xy". Then from the above we determine the
 * (i,j), and with that the associated level level(j,i) = level[j*nx + i].
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
  size_t ii, jj;
  float ix, jy;	/* indices i,j as floats */
  float x, y, lon, lat;
  float cellsize;	/* asc format requires square cell size */
  float dlon, dlat;
  float rlon_min, rlat_min, rlon_max, rlat_max;

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
    if(sscanf(llur_str, "%f,%f,%f,%f",
	      &rlon_min, &rlat_min, &rlon_max, &rlat_max) != 4){
      log_errx(0, "Invalid value of enclosing rectangle limits");
      return(1);
    }

    if(f_llur_str_diff != 0){
      /* shrink the rectangle by the specified amounts */
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
   * Since we (may) have redefined the limits, and redefined the cellsize
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
   * Then for that (gm_lon(k),gm_lat(k)) we calculate the x, y
   * using lonlat2xy(), and then determine the (ii,jj) in the
   * original grid by
   *
   *  ii = (1/dx) * (x - x_min)
   *  jj = (1/dy) * (y_max - y)
   *
   * and set
   *
   * gm->level[k] = pm->point.level[kk]  (kk = jj* nx + i)
   */
   
  for(j = 0; j < gmap->nlat; ++j){
    for(i = 0; i < gmap->nlon; ++i){
      lon = gmap->lon_min + cellsize * i;
      lat = gmap->lat_max - cellsize * j;

      /* convert to radians */
      lon *= RAD_PER_DEG;
      lat *= RAD_PER_DEG;
      lonlat2xy(lon, lat, &x, &y, pmap->lorigin);

      /*
       * Calculate the indices corresponding to these values x,y, as float
       */
      ix = (1.0/pmap->dx) * (x - pmap->x_min);
      jy = (1.0/pmap->dy) * (pmap->y_max - y);

      if((ix < 0.0) || (ix > (float)pmap->nx - 1.0))
	*datap = DCGOESR_GRID_MAP_NODATA;
      else if((jy < 0.0) || (jy > (float)pmap->ny - 1.0))
	*datap = DCGOESR_GRID_MAP_NODATA;      
      else{
	ii = LROUND_SIZE_T(ix);
	jj = LROUND_SIZE_T(jy);
	*datap = (int)pmap->points[jj * pmap->nx + ii].level;
      }
      ++datap;
    }
  }

  return(0);
}

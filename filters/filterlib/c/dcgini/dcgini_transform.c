/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdio.h>	/* XXX - only for debugging */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>	/*  SIZE_T_MAX */
#include "err.h"
#include "dcgini_const.h"
#include "dcgini_transform.h"
#include "dcgini_transform_priv.h"

static void dcgini_pointmap_bb(struct dcgini_point_map_st *pm);

int dcgini_transform_data(struct dcgini_st *dcg){

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
    nesdis_proj_str_init(&dcg->pdb, &dcg->pstr);
  else if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_LLC)
    nesdis_proj_llc_init(&dcg->pdb, &dcg->pllc);
  else if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_MER)
    nesdis_proj_mer_init(&dcg->pdb, &dcg->pmer);
  else{
    log_warnx("Unsupported map projection: %d", dcg->pdb.map_projection);
    return(1);
  }

  /* Allocate storage for the pointmap */
  numpoints = dcg->pdb.nx * dcg->pdb.ny;

  if(numpoints > dcg->ginidata.data_size){
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

  /* Re-count the number of points after filtering */
  numpoints = 0;

  /*
   * Assume the scan is top to bottom, left to right; in principle
   * we should check the pdb.scan_mode flag.
   */
  for(j = dcg->pdb.ny - 1; j >= 0; --j){
    for(i = 0; i < dcg->pdb.nx; ++i){
      if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_STR)
	nesdis_proj_str_ij_lonlat(&dcg->pstr, i, j, &lon_deg, &lat_deg);
      else if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_LLC)
	nesdis_proj_llc_ij_lonlat(&dcg->pllc, i, j, &lon_deg, &lat_deg);
      else if(dcg->pdb.map_projection == NESDIS_MAP_PROJ_MER)
	nesdis_proj_mer_ij_lonlat(&dcg->pmer, i, j, &lon_deg, &lat_deg);

      /*
       * We are not applying any filter so we insert the point
       * unconditionally.
       */
      points->lon = lon_deg;
      points->lat = lat_deg;
      points->level = (int)*datap;
      ++points;
      ++numpoints;
      ++datap;
    }
  }

  dcg->pointmap.numpoints = numpoints;
  dcgini_pointmap_bb(&dcg->pointmap);

  /* XXX
  fprintf(stdout, "%f %f %f %f %f %f\n",
	  dcg->pdb.lon1_deg,
	  dcg->pdb.lat1_deg,
	  dcg->pdb.lon2_deg,
	  dcg->pdb.lat2_deg,
	  dcg->pdb.lon_ur_deg,
	  dcg->pdb.lat_ur_deg);
  exit(0);
  */

  return(0);
}

static void dcgini_pointmap_bb(struct dcgini_point_map_st *pm){
  /*
   * This function determines the boundinx box that limits of the raw data.
   * as well as the smallest rectangle that encloses
   * all data points, i.e., points for which the level is not zero.
   */
  size_t i;

  pm->lon_min = 180.0;
  pm->lon_max = -180.0;
  pm->lat_min = 180.0;
  pm->lon_max = -180.0;

  pm->lon_ll = pm->lon_min;
  pm->lat_ll = pm->lat_min;
  pm->lon_ur = pm->lon_max;
  pm->lat_ur = pm->lat_max;
  
  for(i = 0; i < pm->numpoints; ++i){
    if(pm->points[i].lon < pm->lon_min)
      pm->lon_min = pm->points[i].lon;

    if(pm->points[i].lon > pm->lon_max)
      pm->lon_max = pm->points[i].lon;

    if(pm->points[i].lat < pm->lat_min)
      pm->lat_min = pm->points[i].lat;

    if(pm->points[i].lat > pm->lat_max)
      pm->lat_max = pm->points[i].lat;

    /* exclude background points in the determination of the limits */
    if(pm->points[i].level == 0)
      continue;

    if(pm->points[i].lon < pm->lon_ll)
      pm->lon_ll = pm->points[i].lon;

    if(pm->points[i].lon > pm->lon_ur)
      pm->lon_ur = pm->points[i].lon;

    if(pm->points[i].lat < pm->lat_ll)
      pm->lat_ll = pm->points[i].lat;

    if(pm->points[i].lat > pm->lat_ur)
      pm->lat_ur = pm->points[i].lat;
  }

  /* XXX
  fprintf(stdout, "%f %f %f %f\n", pm->lon_min, pm->lat_min,
	  pm->lon_max, pm->lat_max);

  fprintf(stdout, "%f %f %f %f\n", pm->lon_ll, pm->lat_ll,
	  pm->lon_ur, pm->lat_ur);
  ***/
}

int dcgini_regrid_data(struct dcgini_st *dcg){
  /*
   * In this function, dlon and dlat are not the the same; i.e.,
   * the cells are not squared. The cells in the dcgini_regrid_data_asc()
   * function are squared (as required by the asc format).
   *
   * This function assumes that dcgini_transform_data() has already been
   * called, so that the projection has been initialized, and in particular
   * the bounding box has been determined. Note that we do not use
   * the lon1/lat1, lon2/lat2 values from the pdb because that would cut
   * portions of the image.
   */
  size_t numpoints;
  int *datap;
  size_t i, j, k, l;
  double ii, jj, lon_deg, lat_deg;

  /* Allocate storage for the levels array */
  numpoints = dcg->pdb.nx * dcg->pdb.ny;

  if(numpoints > dcg->ginidata.data_size){
    log_warnx("The data size is smaller than nx * ny");
    return(1);
  }

  dcg->gridmap.level = calloc(numpoints, sizeof(*dcg->gridmap.level));
  if(dcg->gridmap.level == NULL){
    log_err(1, "Cannot allocate memory for gridmap");
    return(-1);
  }

  dcg->gridmap.numpoints = numpoints;
  dcg->gridmap.nlon = dcg->pdb.nx;
  dcg->gridmap.nlat = dcg->pdb.ny;
  dcg->gridmap.lon1_deg = dcg->pointmap.lon_min;
  dcg->gridmap.lat1_deg = dcg->pointmap.lat_min;
  dcg->gridmap.lon2_deg = dcg->pointmap.lon_max;
  dcg->gridmap.lat2_deg = dcg->pointmap.lat_max;
  dcg->gridmap.dlon_deg = 
    (dcg->gridmap.lon2_deg - dcg->gridmap.lon1_deg)/(dcg->gridmap.nlon - 1);
  dcg->gridmap.dlat_deg =
    (dcg->gridmap.lat2_deg - dcg->gridmap.lat1_deg)/(dcg->gridmap.nlat - 1);

  datap = dcg->gridmap.level;

  /*
   * Store the values in the order top to bottom, right to left,
   * similar to the ArcInfo ASCII Grid format (see the regrid_asc function).
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
	i = (size_t)lround(ii);
	j = dcg->pdb.ny - 1 - (size_t)lround(jj); /* from top to bottom */
	*datap = (int)dcg->ginidata.data[j * dcg->pdb.nx + i];
      }
      ++datap;
    }
  }

  return(0);
}

int dcgini_regrid_data_asc(struct dcgini_st *dcg,
			   char *llur_str,
			   int f_llur_str_diff){
  /*
   * The distinctive feature in this function is that here we define
   * the cellsize (dlon, dlat) such that it is a square cell, as 
   * required by the "asc" format.
   *
   * This function assumes that dcgini_transform_data() has already been
   * called, so that the projection has been initialized, and in particular
   * the bounding box and the maximum enclosing rectangle have been determined.
   */
  size_t numpoints;
  int *datap;
  size_t i, j, k, l;
  double ii, jj, lon_deg, lat_deg;
  double cellsize;	/* asc format requires square cell size */
  double dlon, dlat;
  double rlon1, rlat1, rlon2, rlat2;

  dlon = (dcg->pointmap.lon_max - dcg->pointmap.lon_min)/(dcg->pdb.nx - 1);
  dlat = (dcg->pointmap.lat_max - dcg->pointmap.lat_min)/(dcg->pdb.ny - 1);
  if(dlat < dlon)
    cellsize = dlat;
  else
    cellsize = dlon;

  dcg->gridmap.dlon_deg = cellsize;
  dcg->gridmap.dlat_deg = cellsize;
  dcg->gridmap.cellsize_deg = cellsize;

  /* the default limits */
  dcg->gridmap.lon1_deg = dcg->pointmap.lon_ll;
  dcg->gridmap.lat1_deg = dcg->pointmap.lat_ll;
  dcg->gridmap.lon2_deg = dcg->pointmap.lon_ur;
  dcg->gridmap.lat2_deg = dcg->pointmap.lat_ur;

  /* optional limits */
  if(llur_str != NULL){
    if(sscanf(llur_str, "%lf,%lf,%lf,%lf",
	      &rlon1, &rlat1, &rlon2, &rlat2) != 4){
      log_errx(1, "Invalid value of enclosing rectangle limits");
      return(1);
    }

    if(f_llur_str_diff != 0){
      /* shrink the rectangle by the specified amouns */
      dcg->gridmap.lon1_deg += rlon1;
      dcg->gridmap.lat1_deg += rlat1;
      dcg->gridmap.lon2_deg -= rlon2;
      dcg->gridmap.lat2_deg -= rlat2;
    }else{
      dcg->gridmap.lon1_deg = rlon1;
      dcg->gridmap.lat1_deg = rlat1;
      dcg->gridmap.lon2_deg = rlon2;
      dcg->gridmap.lat2_deg = rlat2;
    }
  }

  dcg->gridmap.nlon =
    (dcg->gridmap.lon2_deg - dcg->gridmap.lon1_deg)/cellsize + 1.0;
  dcg->gridmap.nlat =
    (dcg->gridmap.lat2_deg - dcg->gridmap.lat1_deg)/cellsize + 1.0;

  /* Allocate storage for the levels array */
  numpoints = dcg->gridmap.nlon * dcg->gridmap.nlat;

  /* XXX
  fprintf(stdout, "%f %f", dcg->gridmap.lat2_deg, dcg->gridmap.lat1_deg);
  fprintf(stdout, "%u %u\n", dcg->gridmap.nlon, dcg->gridmap.nlat);
  */

  dcg->gridmap.level = calloc(numpoints, sizeof(*dcg->gridmap.level));
  if(dcg->gridmap.level == NULL){
    log_err(1, "Cannot allocate memory for gridmap");
    return(-1);
  }

  datap = dcg->gridmap.level;
  dcg->gridmap.numpoints = numpoints;

  /*
   * Store the values in the order defined by the ArcInfo ASCII Grid format:
   * the origin of the grid is the upper left and terminus at the lower right.
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
	i = (size_t)lround(ii);
	j = dcg->pdb.ny - 1 - (size_t)lround(jj);    /* from top to bottom */
	*datap = (int)dcg->ginidata.data[j * dcg->pdb.nx + i];
      }
      ++datap;
    }
  }

  return(0);
}

void nesdis_proj_str_init(struct nesdis_pdb_st *npdb,
			  struct nesdis_proj_str_st *pstr){
  double r;

  pstr->s = 1.0;
  if(npdb->proj_center_flag != 0){
    pstr->s = -1.0;
  }
  pstr->lov_rad = npdb->lov_rad;
  pstr->alpha = 1.0/(1.0 + sin(M_PI/3.0));

  r = RE_METERS * tan(M_PI_4 - 0.5 * pstr->s * npdb->lat1_rad);
  pstr->x1 = r * sin(npdb->lon1_rad - pstr->lov_rad);
  pstr->y1 = -pstr->s * r * cos(npdb->lon1_rad - pstr->lov_rad);
  pstr->dx = pstr->alpha * npdb->dx_meters;
  pstr->dy = pstr->alpha * npdb->dy_meters;

  /*
   * Update the lon2/lat2 values in the pdb (see note in fill_nesdis_pdb()
   * in dcgini_pdb.c.
   */

  /*
   * The i, j coordinates of the "last" (ur) grid point are
   * i = dcg->pdb.nx - 1;
   * j = dcg->pdb.ny - 1;;
   */
  nesdis_proj_str_ij_lonlat(pstr, npdb->nx - 1, npdb->ny - 1,
			    &npdb->lon2_deg, &npdb->lat2_deg);
  npdb->lon2_rad = npdb->lon2_deg * RAD_PER_DEG;
  npdb->lat2_rad = npdb->lat2_deg * RAD_PER_DEG;
  npdb->lon_ur_deg = npdb->lon2_deg;
  npdb->lat_ur_deg = npdb->lat2_deg;
  npdb->lon_ur_rad = npdb->lon2_rad;
  npdb->lat_ur_rad = npdb->lat2_rad;
}

void nesdis_proj_str_ij_lonlat(struct nesdis_proj_str_st *pstr,
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
  lat_rad = pstr->s * (M_PI_2 - 2.0 * atan2(r, RE_METERS));
  
  *lon_deg = lon_rad * DEG_PER_RAD;
  *lat_deg = lat_rad * DEG_PER_RAD;
}

void nesdis_proj_str_lonlat_ij(struct nesdis_proj_str_st *pstr,
			       double lon_deg,
			       double lat_deg,
			       double *ii,
			       double *jj){
  double lon_rad, lat_rad;
  double x, y;
  double r;

  lon_rad = lon_deg * RAD_PER_DEG;
  lat_rad = lat_deg * RAD_PER_DEG;

  r = RE_METERS * tan(M_PI_4 - 0.5 * pstr->s * lat_rad);
  x = r * sin(lon_rad - pstr->lov_rad);
  y = -pstr->s * r * cos(lon_rad - pstr->lov_rad);

  *ii = (x - pstr->x1)/pstr->dx;
  *jj = (y - pstr->y1)/pstr->dy;
}

void nesdis_proj_llc_init(struct nesdis_pdb_st *npdb,
			  struct nesdis_proj_llc_st *pllc){
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
  pllc->alpha = pow(tan(psi/2.0), pllc->cos_psi) / sin(psi);

  a = pow(tan(M_PI_4 - 0.5 * pllc->s * npdb->lat1_rad), pllc->cos_psi);
  b = pllc->cos_psi * (npdb->lon1_rad - pllc->lov_rad);
  pllc->x1 = pllc->r_E * a * sin(b);
  pllc->y1 = -pllc->s * pllc->r_E * a * cos(b);
  pllc->dx = pllc->alpha * npdb->dx_meters;
  pllc->dy = pllc->alpha * npdb->dy_meters;

  /*
  fprintf(stdout, "%f %f %f %f %f\n", pllc->x1, pllc->y1, pllc->dx, pllc->dy,
  pllc->s);
  exit(0);
  */

  /*
   * Update the lon2/lat2 values in the pdb (see note in fill_nesdis_pdb()
   * in dcgini_pdb.c.
   */

  /*
   * The i, j coordinates of the "last" (ur) grid point are
   * i = dcg->pdb.nx - 1;
   * j = dcg->pdb.ny - 1;
   */
  nesdis_proj_llc_ij_lonlat(pllc, npdb->nx - 1, npdb->ny - 1,
			    &npdb->lon2_deg, &npdb->lat2_deg);
  npdb->lon2_rad = npdb->lon2_deg * RAD_PER_DEG;
  npdb->lat2_rad = npdb->lat2_deg * RAD_PER_DEG;
  npdb->lon_ur_deg = npdb->lon2_deg;
  npdb->lat_ur_deg = npdb->lat2_deg;
  npdb->lon_ur_rad = npdb->lon2_rad;
  npdb->lat_ur_rad = npdb->lat2_rad;
}

void nesdis_proj_llc_ij_lonlat(struct nesdis_proj_llc_st *pllc,
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
}


void nesdis_proj_llc_lonlat_ij(struct nesdis_proj_llc_st *pllc,
			       double lon_deg,
			       double lat_deg,
			       double *ii,
			       double *jj){
  double lon_rad, lat_rad;
  double x, y;
  double a, b;

  lon_rad = lon_deg * RAD_PER_DEG;
  lat_rad = lat_deg * RAD_PER_DEG;

  a = pow(tan(M_PI_4 - 0.5 * pllc->s * lat_rad), pllc->cos_psi);
  b = pllc->cos_psi * (lon_rad - pllc->lov_rad);
  x = pllc->r_E * a * sin(b);
  y = -pllc->s * pllc->r_E * a * cos(b);

  *ii = (x - pllc->x1)/pllc->dx;
  *jj = (y - pllc->y1)/pllc->dy;
}

void nesdis_proj_mer_init(struct nesdis_pdb_st *npdb,
			  struct nesdis_proj_mer_st *pmer){

  double r1, r2;
  double b, d;
  double n;

  /* center longitude ? */
  pmer->lon0_rad = 0.5 * (npdb->lon1_rad + npdb->lon2_rad);

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

void nesdis_proj_mer_ij_lonlat(struct nesdis_proj_mer_st *pmer,
			       int i,
			       int j,
			       double *lon_deg,
			       double *lat_deg){
  double x, y;
  double lon_rad, lat_rad;

  x = pmer->x1 + (double)i * pmer->dx;
  y = pmer->y1 + (double)j * pmer->dy;

  lon_rad = pmer->lon0_rad + atan2(sinh(x), cos(y));
  lat_rad = asin(sin(y)/cosh(x));

  *lon_deg = lon_rad * DEG_PER_RAD;
  *lat_deg = lat_rad * DEG_PER_RAD;
}

void nesdis_proj_mer_lonlat_ij(struct nesdis_proj_mer_st *pmer,
			       double lon_deg,
			       double lat_deg,
			       double *ii,
			       double *jj){
  double lon_rad, lat_rad;
  double r1, r2;
  double b, d;
  double x, y;

  lon_rad = lon_deg * RAD_PER_DEG;
  lat_rad = lat_deg * RAD_PER_DEG;

  d = lon_rad - pmer->lon0_rad;
  r1 = tan(lat_rad);
  r2 = cos(d);
  b = cos(lat_rad) * sin(d);  
  x = 0.5 * log((1.0 + b)/(1.0 - b));
  y = atan2(r1, r2);

  *ii = (x - pmer->x1)/pmer->dx;
  *jj = (y - pmer->y1)/pmer->dy;
}

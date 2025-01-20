/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <math.h>
#include <netcdf.h>
#include "dcgoesr_nc.h"
#include "dcgoesr_xy2lonlat.h"
#include "dcgoesr_cmilevel.h"

/* private functions */
static int get_offset_scale(int ncid, int varid, double *offset, double *scale);
static int get_xy(int ncid, int varid, double *v, int nv);
static int get_cmi(int ncid, int varid, double *cmip, int Npoints);
static int get_lonorigin(int ncid, double *lorigin);
static int get_tclonlat(int ncid, double *lon, double *lat);
static void calc_boundingbox(double *lon, double *lat, int Npoints,
			     double *lon1, double *lat1,
			     double *lon2, double *lat2);

/* variable names */
#define XNAME "x"
#define YNAME "y"
#define CMI_NAME "Sectorized_CMI"
#define CMI_PROJECTION_NAME "fixedgrid_projection"
#define CMI_LONORIGIN_NAME "longitude_of_projection_origin"
#define RAD_NAME "Rad"
#define RAD_PROJECTION_NAME "goes_imager_projection"
#define RAD_LONORIGIN_NAME "longitude_of_projection_origin"
/* The glm "Flash_extent_density" is not unused; not a physical variable */
#define GLM_NAME "Total_Optical_energy"
#define GLM_PROJECTION_NAME "goes_imager_projection"
#define GLM_LONORIGIN_NAME "longitude_of_projection_origin"

/* the global "attributes */
#define CMI_TC_LONGITUDE "tile_center_longitude"   /* not in all files */
#define CMI_TC_LATITUDE "tile_center_latitude"	   /* not in all files */
#define RAD_TC_LONGITUDE NULL
#define RAD_TC_LATITUDE NULL
#define GLM_TC_LONGITUDE NULL
#define GLM_TC_LATITUDE NULL

/* the factor to convert the x,y (lon,lat) data to radians */
#define CMI_XY_RADIANS_UNITS_FACTOR 0.000001
#define RAD_XY_RADIANS_UNITS_FACTOR 1.0
#define GLM_XY_RADIANS_UNITS_FACTOR 1.0

/* store the parameters in a static variable and initialize */
static struct {
  char *var_name;
  char *proj_name;
  char *proj_lonorigin_name;
  char *tc_longitude;
  char *tc_latitude;
  double xy_radians_units_factor;
} gdcgoesr = {CMI_NAME, CMI_PROJECTION_NAME, CMI_LONORIGIN_NAME,
	      CMI_TC_LONGITUDE, CMI_TC_LATITUDE, CMI_XY_RADIANS_UNITS_FACTOR};

static int get_offset_scale(int ncid, int varid,
			    double *offset, double *scale) {
  /*
   * Get the "offset" and "scale" parameters of the variable identified
   * by the varid from the nc file.
   */
  int status;
  
  status = nc_get_att_double(ncid, varid, "add_offset", offset);
  if(status == 0)
    status = nc_get_att_double(ncid, varid, "scale_factor", scale);

  /*
   * Not all variables have these two attributes. For example,
   * the glm "Flash_extent_density" in the tirs00 files.
   */

  if(status == NC_ENOTATT){
    *offset = 1.0;
    *scale = 0.0;
    status = 0;
  }

  return(status);
}

static int get_xy(int ncid, int varid, double *v, int nv) {
  /*
   * This function is intended to be used for x or y
   */
  double offset, scale;
  int i;
  int status = 0;

  status = get_offset_scale(ncid, varid, &offset, &scale);
  if(status != 0)
    return(status);
  
  /* Read the data. */
  status = nc_get_var_double(ncid, varid, v);
  if(status != 0)
    return(status);

  /* calculate the real values of x or y */
  for(i = 0; i < nv; ++i) {
    v[i] = (v[i]*scale + offset);
    /* convert to radians */
    if(gdcgoesr.xy_radians_units_factor != 1.0)
      v[i] *= gdcgoesr.xy_radians_units_factor;
  }

  return(status);
}

static int get_cmi(int ncid, int varid, double *cmip, int Npoints) {
  /*
   * This function is intended to be used for the cmi.
   */
  double offset, scale;
  int k;
  int status = 0;

  status = get_offset_scale(ncid, varid, &offset, &scale);
  if(status != 0)
    return(status);

  /* Read the data. */
  status = nc_get_var_double(ncid, varid, cmip);
  if(status != 0)
    return(status);

  /* calculate the real data */
  for(k = 0; k < Npoints; ++k) {
    cmip[k] = cmip[k]*scale + offset;
  }

  return(status);
}

static int get_lonorigin(int ncid, double *lorigin) {

  int varid;
  int status;

  status = nc_inq_varid(ncid, gdcgoesr.proj_name, &varid);
  if(status == 0) {
    status = nc_get_att_double(ncid, varid, gdcgoesr.proj_lonorigin_name,
			     lorigin);
  }

  return(status);
}

static int get_tclonlat(int ncid, double *lon, double *lat) {
  /*
   * Get the "tile_center_longitude" and latitude. Apart from those files
   * that we have already configured (OR_ABI and glm) some of the goesr
   * files do not have it (e.g., tirs)
   */
  int status = 0;

  if((gdcgoesr.tc_longitude == NULL) || (gdcgoesr.tc_latitude == NULL))
    return(0);
     
  status = nc_get_att_double(ncid, NC_GLOBAL, gdcgoesr.tc_longitude, lon);
     
  if(status == 0)
    status = nc_get_att_double(ncid, NC_GLOBAL, gdcgoesr.tc_latitude, lat);

  /* Not all files have these attributes */
  if(status == NC_ENOTATT) {
    *lon = 0.0;
    *lat = 0.0;
    status = 0;
  }

  return(status);
}

static void calc_boundingbox(double *lon, double *lat, int Npoints,
			     double *lon1, double *lat1,
			     double *lon2, double *lat2) {
  /*
   * Some of the lon,lat points can be Nan (e.g., in goes-16),
   * when the satellite is pointing to space and not to Earth.
   * Use isnan() to exclude them.
   */  
  int i;
  
  double lon_max, lon_min, lat_min, lat_max;

  lon_max = -2.0*M_PI;
  lon_min = 2.0*M_PI;
  lat_max = -2.0*M_PI;
  lat_min = 2.0*M_PI;

  for(i = 0; i < Npoints; ++i) {
    if(isnan(lon[i]))
      continue;

    if(lon[i] > lon_max)
      lon_max = lon[i];
    
    if(lon[i] < lon_min)
      lon_min = lon[i];
  }
  
  for(i = 0; i < Npoints; ++i) {
    if(isnan(lat[i]))
      continue;

    if(lat[i] > lat_max)
      lat_max = lat[i];

    if(lat[i] < lat_min)
      lat_min = lat[i];
  }

  *lon1 = lon_min;
  *lat1 = lat_min;
  *lon2 = lon_max;
  *lat2 = lat_max;
}

/* public functions */
void goesr_config(int c) {
  /*
   * Choose std noaaport type (c = 0), glm (c = 1), OR (c = 2) type file
   */
  if(c == 0)
    return;

  if(c == 1) {
    gdcgoesr.var_name = GLM_NAME;
    gdcgoesr.proj_name = GLM_PROJECTION_NAME;
    gdcgoesr.proj_lonorigin_name = GLM_LONORIGIN_NAME;
    gdcgoesr.tc_longitude = GLM_TC_LONGITUDE;
    gdcgoesr.tc_latitude = GLM_TC_LATITUDE;
    gdcgoesr.xy_radians_units_factor = GLM_XY_RADIANS_UNITS_FACTOR;
  }

  if(c == 2) {
    gdcgoesr.var_name = RAD_NAME;
    gdcgoesr.proj_name = RAD_PROJECTION_NAME;
    gdcgoesr.proj_lonorigin_name = RAD_LONORIGIN_NAME;
    gdcgoesr.tc_longitude = RAD_TC_LONGITUDE;
    gdcgoesr.tc_latitude = RAD_TC_LATITUDE;
    gdcgoesr.xy_radians_units_factor = RAD_XY_RADIANS_UNITS_FACTOR;
  }
}

int goesr_create(int ncid, struct goesr_st **goesr) {

  struct goesr_st *gp;
  int xid, yid, cmiid, xdimid, ydimid;
  int nx, ny;		/* size of x and y */
  int Npoints;		/* nx*ny */
  size_t data_size;	/* total size of the data */
  size_t ndim;		/* for getting nx, ny from nc functions */ 
  int i, j;		/* loop indexes x[i], y[j] */
  int k;		/* "cmi(j,i)"  = cmi[k] with k = j*nx + i */
  double lon, lat, lorigin;	/* for the conversion x,y -> lon,lat */
  int status;

  /*
   * First of all get the sizes nx and ny
   */
  status = nc_inq_dimid(ncid, XNAME, &xdimid);
  if(status == 0) {
    status = nc_inq_dimlen(ncid, xdimid, &ndim);
    if(status == 0)
      nx = (int)ndim;
  }

  if(status != 0)
    return(status);
  
  status = nc_inq_dimid(ncid, YNAME, &ydimid);
  if(status == 0) {
    status = nc_inq_dimlen(ncid, ydimid, &ndim);
    if(status == 0)
      ny = (int)ndim;
  }

  if(status != 0)
    return(status);

  /* Now the id's of x,y,cmi we need */
  status = nc_inq_varid(ncid, XNAME, &xid);
  if(status == 0)
    status = nc_inq_varid(ncid, YNAME, &yid);

  if(status == 0)
    status = nc_inq_varid(ncid, gdcgoesr.var_name, &cmiid);

  /* for the x,y to lon,lat conversion */
  if(status == 0)
    status = get_lonorigin(ncid, &lorigin);

  if(status != 0)
    return(status);

  /* Create the main goers pointer, and the data storage */
  gp = malloc(sizeof(struct goesr_st));
  if(gp == NULL)
    return(-1);

  /* See dcgoesr_nc.h for data_size */
  Npoints = nx*ny;	
  data_size = sizeof(double)*(nx + ny + 3*Npoints) + sizeof(uint8_t)*Npoints;
  gp->data = malloc(data_size);
  if(gp->data == NULL) {
    free(gp);
    return(-1);
  }

  /* Make the various pointers point to the correct place */
  gp->nx = nx;
  gp->ny = ny;
  gp->Npoints = Npoints;
  gp->data_size = data_size;
  gp->lorigin = lorigin;
  /* gp->data is already set */
  gp->x = &gp->data[0];
  gp->y = &gp->x[nx];
  gp->cmi = &gp->y[ny];
  gp->lon = &gp->cmi[Npoints];
  gp->lat = &gp->lon[Npoints];
  gp->level = (uint8_t*)&gp->lat[Npoints];

  /* Initialize the global (info) parameters */
  gp->tclon = 0.0;
  gp->tclat = 0.0;
  gp->lon1 = 0.0;
  gp->lat1 = 0.0;
  gp->lon2 = 0.0;
  gp->lat2 = 0.0;

  /* Extract the data variables */
  
  status = get_xy(ncid, xid, gp->x, nx);
  if(status == 0)
    status = get_xy(ncid, yid, gp->y, ny);
  
  if(status == 0)
    status = get_cmi(ncid, cmiid, gp->cmi, Npoints);

  if(status != 0) {
    goesr_free(gp);
    return(status);
  }

  /* calculate lon, lat */
  for (j = 0; j < ny; ++j) {
    for (i = 0; i < nx; ++i) {
      k = j*gp->nx + i;  /* this is the index of the corresponding cmi[k] */
      xy2lonlat(gp->x[i], gp->y[j], &lon, &lat, lorigin);
      gp->lon[k] = lon;
      gp->lat[k] = lat;
    }
  }

  /* calculate the normalized "level" values */
  cmilevel(gp->cmi, gp->level, Npoints);
  
  /*
   * Now all the "global" nc attributes, and our global parameters
   * (lower-left and upper-right coordinates)
   */
  status = get_tclonlat(ncid, &gp->tclon, &gp->tclat);
  if(status != 0) {
    goesr_free(gp);
    return(status);
  }

  /*
   * Determine the "boundig box"
   */
  calc_boundingbox(gp->lon, gp->lat, gp->Npoints,
		   &gp->lon1, &gp->lat1, &gp->lon2, &gp->lat2);

  *goesr = gp;
  
  return(status);
}

void goesr_free(struct goesr_st *goesr) {
  
  if(goesr == NULL)
    return;

  if(goesr->data != NULL)
    free(goesr->data);

  free(goesr);
}

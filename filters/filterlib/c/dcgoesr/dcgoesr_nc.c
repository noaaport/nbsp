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
#include <math.h>	/* isnan */
#include <float.h>	/* FLT_MAX */
#include <netcdf.h>
#include "dcgoesr_nc.h"
#include "dcgoesr_xy2lonlat.h"

/* private functions */
static int get_offset_scale(int ncid, int varid, double *offset, double *scale);
static int get_xy(int ncid, int varid, double *v, int nv);
static int get_cmi(int ncid, int varid, double *cmip, int Npoints);
static int get_lonorigin(int ncid, double *lorigin);
static int get_tclonlat(int ncid, double *lon, double *lat);
static void cmilevel(struct goesr_st *gp);
static void calc_boundingbox(struct goesr_st *gp);

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

static void cmilevel(struct goesr_st *gp) {
  /*
   * This function calculates the "normalized" cmi.
   */
  int k;
  double cmi_max, cmi_min, norm, cmi_normalized;
  double *cmi = gp->cmi;
  int Npoints = gp->Npoints;

  /* determine the max and min */
  cmi_max = 0.0;
  cmi_min = FLT_MAX;
    
  for(k = 0; k < Npoints; ++k) {
    if(cmi[k] > cmi_max)
      cmi_max = cmi[k];

    if(cmi[k] < cmi_min)
      cmi_min = cmi[k];
  }

  /* determine the normalized values */
  norm = 255.0/(cmi_max - cmi_min);
  
  for(k = 0; k < Npoints; ++k) {
    cmi_normalized = (cmi[k] - cmi_min) * norm;
    gp->level[k] = (uint8_t)cmi_normalized;
    gp->pmap.points[k].level = (uint8_t)cmi_normalized; /* a copy */
  }
}

static void calc_boundingbox(struct goesr_st *gp) {
  /*
   * This function determines the "bounding box" (smallest rectangle
   * that encloses the raw data) and the "maximum enclosing rectangle"
   * (excludes background points (level 0) in the determination
   * of the limits).
   * Some of the lon,lat points can be Nan (e.g., in goes-16),
   * when the satellite is pointing to space and not to Earth.
   * Use isnan() to exclude them.
   */
  struct dcgoesr_point_map_st *pm = &gp->pmap;
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
    if(isnan(pm->points[i].lon) || isnan(pm->points[i].lat))
      continue;

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
  data_size = sizeof(double)*(nx + ny + Npoints) + sizeof(uint8_t)*Npoints;
  gp->data = malloc(data_size);

  /* The transformed data */
  gp->pmap.points = malloc(sizeof(struct dcgoesr_point_st) * Npoints);

  /*
   * Initialize, but at this point we don't know yet how many grid points.
   * This is done in "dcgoesr_regrid_data_asc()" (if the asc regrid is
   * requested).
   */
  gp->gmap.level = NULL;

  if((gp->data == NULL) || (gp->pmap.points == NULL)) {
    goesr_free(gp);
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
  gp->level = (uint8_t*)&gp->cmi[Npoints];

  /* Initialize the global (info) parameters */
  gp->tclon = 0.0;
  gp->tclat = 0.0;

  /* all min,max are determined by calc_boundingbox() below */
  gp->pmap.numpoints = Npoints;
  gp->pmap.lon_min = 0.0;
  gp->pmap.lat_min = 0.0;
  gp->pmap.lon_max = 0.0;
  gp->pmap.lat_max = 0.0;
  gp->pmap.lon_ll = gp->pmap.lon_min;
  gp->pmap.lat_ll = gp->pmap.lat_min;
  gp->pmap.lon_ur = gp->pmap.lon_max;
  gp->pmap.lat_ur = gp->pmap.lat_max;
  gp->pmap.nx = nx;
  gp->pmap.ny = ny;
  gp->pmap.lorigin = gp->lorigin;
  /* These are set below
  gp->pmap.x_min
  gp->pmap.x_max
  gp->pmap.y_min
  gp->pmap.y_max
  */

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

  /*
   * Now all the "global" nc attributes,
   */
  status = get_tclonlat(ncid, &gp->tclon, &gp->tclat);
  if(status != 0) {
    goesr_free(gp);
    return(status);
  }

  /*
   * Our conversions and global parameters
   */

  /* calculate lon, lat */
  for (j = 0; j < ny; ++j) {
    for (i = 0; i < nx; ++i) {
      k = j*gp->nx + i;  /* this is the index of the corresponding cmi[k] */
      xy2lonlat(gp->x[i], gp->y[j], &lon, &lat, lorigin); /* lon,lat in rads */
      gp->pmap.points[k].lon = lon*DEG_PER_RAD;
      gp->pmap.points[k].lat = lat*DEG_PER_RAD;
    }
  }

  /* (these are used in the regrid asc) */
  gp->pmap.x_min = gp->x[0];
  gp->pmap.x_max = gp->x[nx - 1];
  gp->pmap.y_min = gp->y[ny - 1];	/* top to bottom */
  gp->pmap.y_max = gp->y[0];
  gp->pmap.dx = (gp->pmap.x_max - gp->pmap.x_min)/((double)gp->pmap.nx - 1.0);
  gp->pmap.dy = (gp->pmap.y_max - gp->pmap.y_min)/((double)gp->pmap.ny - 1.0);

  /* calculate the normalized "level" values */
  cmilevel(gp);

  /*
   * Determine the "bounding box" and "maximum enclosing rectangle"
   */
  calc_boundingbox(gp);




  *goesr = gp;
  
  return(status);
}

void goesr_free(struct goesr_st *goesr) {
  
  if(goesr == NULL)
    return;

  if(goesr->gmap.level != NULL)
    free(goesr->gmap.level);  

  if(goesr->pmap.points != NULL)
    free(goesr->pmap.points);

  if(goesr->data != NULL)
    free(goesr->data);

  free(goesr);
}

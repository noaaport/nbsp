#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <math.h>
#include <netcdf.h>
#include "dcgoesr_nc.h"
#include "dcgoesr_xy2lonlat.h"

/* private functions */
static int get_offset_scale(int ncid, int varid, double *offset, double *scale);
static int get_xy(int ncid, int varid, double *v, int nv);
static int get_cmi(int ncid, int varid, double *cmip, int nx, int ny);
static int get_lonorigin(int ncid, double *lorigin);

/* constants */
#define XNAME "x"
#define YNAME "y"
#define CMINAME "Sectorized_CMI"

static int get_offset_scale(int ncid, int varid,
			    double *offset, double *scale) {
  /*
   * get the "offset" and "scale" parameters of the variable identified
   * by the varid from the nc file.
   */
  int status;
  
  status = nc_get_att_double(ncid, varid, "add_offset", offset);
  if(status == 0)
    status = nc_get_att_double(ncid, varid, "scale_factor", scale);

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
    v[i] = (v[i]*scale + offset)*0.000001;
  }

  return(status);
}

static int get_cmi(int ncid, int varid, double *cmip, int nx, int ny) {
  /*
   * This function is intended to be used for the cmi
   */
  double offset, scale;
  int k;
  int status = 0;
  int Ndim;

  Ndim = nx*ny;

  status = get_offset_scale(ncid, varid, &offset, &scale);
  if(status != 0)
    return(status);

  /* Read the data. */
  status = nc_get_var_double(ncid, varid, cmip);
  if(status != 0)
    return(status);

  /* calculate the real data */
  for(k = 0; k < Ndim; ++k) {
    cmip[k] = cmip[k]*scale + offset;
  }

  return(status);
}

static int get_lonorigin(int ncid, double *lorigin) {

  int varid;
  int status;

  status = nc_inq_varid(ncid, "fixedgrid_projection", &varid);
  if(status == 0) {
    status = nc_get_att_double(ncid, varid, "longitude_of_projection_origin",
			     lorigin);
  }

  return(status);
}

/* public functions */
int goesr_create(int ncid, struct goesr_st **goesr) {

  struct goesr_st *gp;
  int xid, yid, cmiid, xdimid, ydimid;
  int nx, ny, ndata;	/* size of x and y */
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
    status = nc_inq_varid(ncid, CMINAME, &cmiid);

  /* for the x,y to lon,lat conversion */
  if(status == 0)
    status = get_lonorigin(ncid, &lorigin);
  
  if(status != 0)
    return(status);

  /* Create the main goers pointer, and the data storage */
  gp = malloc(sizeof(struct goesr_st));
  if(gp == NULL)
    return(-1);

  ndata = nx + ny + 3*nx*ny;
  gp->data = malloc(sizeof(double) * ndata);
  if(gp->data == NULL) {
    free(gp);
    return(-1);
  }

  /* Make the various pointers point to the correct place */
  gp->nx = nx;
  gp->ny = ny;
  gp->ndata = ndata;
  gp->lorigin = lorigin;
  /* gp->data is already set */
  gp->x = &gp->data[0];
  gp->y = &gp->x[nx];
  gp->cmi = &gp->y[ny];
  gp->lon = &gp->cmi[nx*ny];
  gp->lat = &gp->lon[nx*ny];  
  
  status = get_xy(ncid, xid, gp->x, nx);
  if(status == 0)
    status = get_xy(ncid, yid, gp->y, ny);
  
  if(status == 0)
    status = get_cmi(ncid, cmiid, gp->cmi, nx, ny);

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

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <math.h>
#include <netcdf.h>

struct {
  int nx;	/* size of x */
  int ny;	/* size of y */
  double *x;
  double *y;
  double *cmi;
} g = {0, 0, NULL, NULL, NULL};
 
/* This is the name of the data file we will read. */
#define FILE_NAME "tire13.nc"
 
/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERRCODE 1
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

void xy2lonlat(double x, double y, double *lon, double *lat,
	       double lorigin);

int get_offset_scale(int ncid, int varid, double *offset, double *scale) {
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

int nc_getxy(int ncid, char *varname, double **v, int *nv) {
  /*
   * This function is intended to be used for x or y
   */
  int varid, dimid;
  size_t ndim;
  int Ndim;	/* same as ndim but as an int */
  double offset, scale;
  double *p;
  int i;
  int status = 0;

  /* Get the varid of the data variable, based on its name (x or y) */
  status = nc_inq_varid(ncid, varname, &varid);
  if(status != 0)
    return(status);

  fprintf(stdout, "varid: %d\n", varid);

  status = nc_inq_dimid(ncid, varname, &dimid);
  if(status != 0)
    return(status);

  fprintf(stdout, "dimid: %d\n", dimid);

  status = nc_inq_dimlen(ncid, dimid, &ndim);
  if(status != 0)
    return(status);

  Ndim = (int)ndim;

  fprintf(stdout, "ndim: %d\n", Ndim);

  status = get_offset_scale(ncid, varid, &offset, &scale);
  if(status != 0)
    return(status);

  fprintf(stdout, "offset:%lf, scale:%lf\n", offset, scale);

  p = malloc(sizeof(double) * Ndim);
  if(p == NULL)
    return(-1);
  
  /* Read the data. */
  status = nc_get_var_double(ncid, varid, p);
  if(status != 0){
    free(p);
    return(status);
  }

  /* calculate the real data */
  for(i = 0; i < Ndim; ++i) {
    p[i] = (p[i]*scale + offset)*0.000001;
  }

  *v = p;
  *nv = Ndim;

  return(status);
}

int nc_getcmi(int ncid, char *varname, double *cmip, int nx, int ny) {
  /*
   * This function is intended to be used for the cmi
   */
  int varid;
  double offset, scale;
  int i, j;
  int status = 0;
  int Ndim;

  Ndim = nx*ny;

  /* Get the varid of the data variable, based on its name */
  status = nc_inq_varid(ncid, varname, &varid);
  if(status != 0)
    return(status);

  fprintf(stdout, "cmi varid: %d\n", varid);

  status = get_offset_scale(ncid, varid, &offset, &scale);
  if(status != 0)
    return(status);

  fprintf(stdout, "cmi offset:%lf, scale:%lf\n", offset, scale);

  /* Read the data. */
  status = nc_get_var_double(ncid, varid, cmip);
  if(status != 0)
    return(status);

  /* calculate the real data */
  for(i = 0; i < Ndim; ++i) {
    cmip[i] = cmip[i]*scale + offset;
  }

  return(status);
}

int main(void){

  int ncid;
  double *x = NULL, *y = NULL;
  int i, j;		/* loop indexes x[i], y[j] */
  int nx, ny;
  double *cmi = NULL;	/* cmi size = nx*ny */
  int k;		/* "cmi(j,i)"  = cmi[k] with k = j*nx + i */
  int status = 0;
  double lon, lat;	/* calculated output, in radians */
  double lorigin = -75.0;
 
  /*
   * Open the file. NC_NOWRITE tells netCDF we want read-only access
   * to the file.
   */
  status = nc_open(FILE_NAME, NC_NOWRITE, &ncid);
  if(status != 0)
    ERR(status);
  
  status = nc_getxy(ncid, "x", &x, &nx);
  if(status == -1)
    err(1, "malloc");
  else if(status != 0)
    ERR(status);
  
  status = nc_getxy(ncid, "y", &y, &ny);
  if(status == -1)
    err(1, "malloc");
  else if(status != 0)
    ERR(status);

  /*
   * start - cmi
   */
  cmi = malloc(sizeof(double) * (nx*ny));
  fprintf(stdout, "B\n");
  if(cmi == NULL)
    err(1, "%s", "malloc");

  status = nc_getcmi(ncid, "Sectorized_CMI", cmi, nx, ny);

  /*
   * end - cmi
   */

  status = nc_close(ncid);
  if(status != 0)
    ERR(status);
  
  /* print in the order x,y, with x varying faster */
  for (j = 0; j < ny; ++j) {
    for (i = 0; i < nx; ++i) {
      k = j*nx + i;	/* "cmi(j,i)"  = cmi[k] with k = j*nx + i */
      /*
       * fprintf(stdout, "%lf,%lf\n", x[i], y[j]);
       */
      xy2lonlat(x[i], y[j], &lon, &lat, lorigin);
      fprintf(stdout, "%f,%f,%f\n", lon, lat, cmi[k]);
    }
  }

  return(0);
}

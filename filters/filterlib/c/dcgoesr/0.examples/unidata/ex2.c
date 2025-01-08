/* https://docs.unidata.ucar.edu/netcdf-c/4.9.2/simple_xy_rd_8c-example.html */

#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
 
/* This is the name of the data file we will read. */
#define FILE_NAME "tire13.nc"
 
/* We are reading 2D data, a 6 x 12 grid. */
#define NX 452
#define NY 476
 
/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}
 
int main(void){
   /* This will be the netCDF ID for the file and data variable. */
   int ncid, varid;
 
   int data_in[NX][NY];
 
   /* Loop indexes, and error handling. */
   int x, y, retval;
 
   /* Open the file. NC_NOWRITE tells netCDF we want read-only access
    * to the file.*/
   if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
      ERR(retval);
 
   /* Get the varid of the data variable, based on its name. */
   if ((retval = nc_inq_varid(ncid, "data", &varid)))
      ERR(retval);
 
   /* Read the data. */
   if ((retval = nc_get_var_int(ncid, varid, &data_in[0][0])))
      ERR(retval);
 
   /* Check the data. */
   for (x = 0; x < NX; x++)
      for (y = 0; y < NY; y++)
     if (data_in[x][y] != x * NY + y)
        return ERRCODE;
 
   /* Close the file, freeing all resources. */
   if ((retval = nc_close(ncid)))
      ERR(retval);
 
   printf("*** SUCCESS reading example file %s!\n", FILE_NAME);
   return 0;
}

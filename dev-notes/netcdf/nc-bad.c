#include <stdlib.h>
#include <err.h>
#include <stdio.h>
#include "netcdf.h"

#define FILENAME "tire.nc"
static int gncid = -1;

static void cleanup(void) {

  int status = 0;

  if(gncid > 0) {
    status = nc_close(gncid);
    fprintf(stdout, "A %d\n", status);
    gncid = -1;
  }
}

int main(int argc, char **argv) {

  int status;

  /* this makes it work */
  /* nc_initialize(); */
  
  atexit(cleanup);
  
  status = nc_open(FILENAME, NC_NOWRITE, &gncid);
  if(status != 0)
    errx(1, "%s", "nc_open");
  
  return(0);
}

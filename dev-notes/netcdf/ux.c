#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h>
#include <stdio.h>

#define FILENAME "tire.nc"
static int gncid = -1;

static void cleanup(void) {

  int status = 0;

  if(gncid > 0) {
    status = close(gncid);
    fprintf(stdout, "A %d\n", status);
    gncid = -1;
  }
}

int main(int argc, char **argv) {

  int status;

  atexit(cleanup);

  gncid = open(FILENAME, O_RDONLY);

  if(gncid == -1)
    err(1, "%s", "open");
  
  return(0);
}

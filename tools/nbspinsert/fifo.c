/*
 * Copyright (c) 2022 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <sys/stat.h>
#include "fifo.h"

int check_fifo(char *fpath) {

  int status = 0;
  int r = 0;
  struct stat sb;

  status = stat(fpath, &sb);
  if(status != 0)
     return(-1);
  
  r = S_ISFIFO(sb.st_mode);
  if(r == 0)
    status = 1;

  return(status);
}

/*
 * Check of isfifo()
 *
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {

  int status = 0;

  if(argc != 2) {
    fprintf(stdout, "%s\n", "Needs arg.");
    exit(1);
  }
  
  status = isfifo(argv[1]);

  if(status == 1)
    fprintf(stdout, "%s\n", "YES");
  else
    fprintf(stdout, "%s\n", "NO");

  return(0);
}
*/

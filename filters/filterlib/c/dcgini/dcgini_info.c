/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdlib.h>
#include <stdio.h>
#include "err.h"
#include "dcgini.h"

int dcgini_info_write(char *file, struct dcgini_point_map_st *pm){

  fprintf(stdout, "%s\n", file);

  return(0);
}

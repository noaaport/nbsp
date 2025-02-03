/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 * Copyright (c) 2025 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dcgoesr_name.h"

char *dcgoesr_name(char *base, char *suffix){

  char *fname;
  int fname_size;
  int n;

  assert(base != NULL);
  if(base == NULL)
    return(NULL);

  fname_size = strlen(base);

  if(suffix != NULL)
    fname_size += strlen(suffix);

  fname = malloc(fname_size + 1);	/* include the '\0' */
  if(fname == NULL)
    return(NULL);

  if(suffix != NULL)
    n = snprintf(fname, fname_size + 1, "%s%s", base, suffix);
  else
    n = snprintf(fname, fname_size + 1, "%s", base);

  assert(n == fname_size);

  return(fname);
}

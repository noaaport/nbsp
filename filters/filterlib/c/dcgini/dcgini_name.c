/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dcgini_name.h"

char *dcgini_default_name(struct nesdis_pdb *npdb,
			  char *prefix,
			  char *suffix){
  /*
   * The base name is the time string defined in dcgini_name.h.
   * The prefix can be NULL, in which case the default (wmoid + underscore,
   * for example "tige01_") is used. The suffix is the file extension,
   * and if it is NULL then nothing is used.
   */
  char *fname;
  int fname_size;
  int n;
  
  if(prefix == NULL)
    prefix = npdb->wmoid;

  if(suffix != NULL)
    n = snprintf(NULL, 0, DCGINI_DEFAULT_NAME_FMT "%s",
		 prefix, 
		 npdb->year, npdb->month, npdb->day, npdb->hour, npdb->min,
		 suffix);
  else
    n = snprintf(NULL, 0, DCGINI_DEFAULT_NAME_FMT,
		 prefix, 
		 npdb->year, npdb->month, npdb->day, npdb->hour, npdb->min);

  fname_size = n;
  fname = malloc(n + 1);    /* includes '\0' */
  if(fname == NULL)
    return(NULL);

  if(suffix != NULL)
    n = snprintf(fname, fname_size + 1, DCGINI_DEFAULT_NAME_FMT "%s",
		 prefix, 
		 npdb->year, npdb->month, npdb->day, npdb->hour, npdb->min,
		 suffix);
  else
    n = snprintf(fname, fname_size + 1, DCGINI_DEFAULT_NAME_FMT,
		 prefix, 
		 npdb->year, npdb->month, npdb->day, npdb->hour, npdb->min);

  assert(n == fname_size);

  return(fname);
}

char *dcgini_optional_name(char *base, char *suffix){

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

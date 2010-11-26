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
  
  fname_size = DCGINI_BASENAME_STR_SIZE;
  if(prefix == NULL)
    prefix = npdb->wmoid;

  fname_size += strlen(prefix);

  if(suffix != NULL)
    fname_size += strlen(suffix);

  fname = malloc(fname_size + 1);	/* include the '\0' */
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

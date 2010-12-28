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
#include "dcnids_name.h"

char *dcnids_default_name(struct nids_header_st *nheader,
			  char *prefix, char *suffix){
  /*
   * The base name is the time string defined in dcnids_name.h.
   * The prefix can be NULL, in which case the default (awips + underscore,
   * for example n0rjua_) is used. The suffix is the file extension,
   * and if it is NULL then nothing is used.
   */
  char *fname;
  int fname_size;
  int n;

  if(prefix == NULL)
    prefix = nheader->awipsid;

  if(suffix != NULL)
    n = snprintf(NULL, 0, DCNIDS_DEFAULT_NAME_FMT "%s",
		 prefix,
		 nheader->year, nheader->month, nheader->day,
		 nheader->hour, nheader->min,
		 suffix);
  else
    n = snprintf(NULL, 0, DCNIDS_DEFAULT_NAME_FMT,
		 prefix,
		 nheader->year, nheader->month, nheader->day,
		 nheader->hour, nheader->min);

  fname_size = n;
  fname = malloc(n + 1);    /* includes '\0' */
  if(fname == NULL)
    return(NULL);

  if(suffix != NULL)
    n = snprintf(fname, fname_size + 1, DCNIDS_DEFAULT_NAME_FMT "%s",
		 prefix,
		 nheader->year, nheader->month, nheader->day,
		 nheader->hour, nheader->min,
		 suffix);
  else
    n = snprintf(fname, fname_size + 1, DCNIDS_DEFAULT_NAME_FMT,
		 prefix,
		 nheader->year, nheader->month, nheader->day,
		 nheader->hour, nheader->min);

  assert(n == fname_size);

  return(fname);
}

char *dcnids_optional_name(char *base, char *suffix){

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

/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <zlib.h>
#include <stdlib.h>
#include "unz.h"

int unz(char *out, int *outlen, char *in, int inlen){

  int e;
  uLong u_outlen = *outlen;
  uLong u_inlen = inlen;

  e = uncompress((Bytef*)out, &u_outlen, (Bytef*)in, u_inlen);
  if(e == Z_OK)
    *outlen = (int)u_outlen;

  return(e);
}

int zip(char **out, int *outlen, char *in, int inlen, int level){

  uLong bound;
  uLong u_inlen = inlen;
  char *p;
  int e;

  bound = compressBound(u_inlen);
  
  p = malloc(bound);
  if(p == NULL)
    return(Z_MEM_ERROR);

  e = compress2((Bytef*)p, &bound, (Bytef*)in, u_inlen, level);
  if(e != Z_OK)
    free(p);
  else{
    *out = p;
    *outlen = bound;
  }

  return(e);
}


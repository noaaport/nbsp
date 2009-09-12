/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "stoi.h"

int strto_int(char *s, int *val){

  char *end;
  int status = 0;
  int v;
  int save_errno;
 
  save_errno = errno;
  errno = 0;

  v = strtol(s, &end, 10);
  if((end == s) || (*end != '\0') || (errno != 0))
    status = 1;
  
  if(status == 0)
    *val = v;

  errno = save_errno;

  return(status);
}

int strto_uint(char *s, unsigned int *val){

  char *end;
  int status = 0;
  unsigned int v;
  int save_errno;
 
  save_errno = errno;
  errno = 0;

  v = strtoul(s, &end, 10);
  if((end == s) || (*end != '\0') || (errno != 0))
    status = 1;
  
  if(status == 0)
    *val = v;

  errno = save_errno;

  return(status);
}

int strto_double(char *s, double *val){

  char *end;
  int status = 0;
  double v;
  int save_errno;
 
  save_errno = errno;
  errno = 0;

  v = strtod(s, &end);
  if( (end == s) || (*end != '\0') )
    status = 1;
  
  if(status == 0)
    *val = v;

  errno = save_errno;

  return(status);
}

int strto_u16(char *s, uint16_t *val){

  unsigned int u;
  int status = 0;

  status = strto_uint(s, &u);
  if((status == 0) && (u <= 0xffff))
      *val = (uint16_t)u;
  else
    status = 1;

  return(status);
}

int strto_u32(char *s, uint32_t *val){

  unsigned int u;
  int status = 0;

  status = strto_uint(s, &u);
  if((status == 0) && (u <= 0xffffffffU))
      *val = (uint32_t)u;
  else
    status = 1;

  return(status);
}

/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <inttypes.h>
#include "dcnids_extract.h"

/*
 * extraction functions
 */
int extract_uint8(unsigned char *p, int halfwordid){
  /*
   * The halfwordid argument is the (1-based) index number as in the Unisys
   * documentation.
   */
  unsigned char r;	/* result */
  unsigned char *b = p;

  b += (halfwordid - 1) * 2;
  r = b[0];
  
  return((int)r);
}

int extract_uint16(unsigned char *p, int halfwordid){
  /*
   * The halfwordid argument is the (1-based) index number as in the Unisys
   * documentation.
   */
  uint16_t r;	/* result */
  unsigned char *b = p;

  b += (halfwordid - 1) * 2;
  r = (b[0] << 8) + b[1];  
  
  return((int)r);
}

uint32_t extract_uint32(unsigned char *p, int halfwordid){

  uint32_t r;	/* result */
  unsigned char *b = p;

  b += (halfwordid - 1) * 2;
  r = (b[0] << 24) + (b[1] << 16) + (b[2] << 8) + b[3];

  return(r);
}

int extract_int16(unsigned char *p, int halfwordid){

  int16_t r;	/* result */
  unsigned char *b = p;

  b += (halfwordid - 1) * 2;
  r = (b[0] << 8) + b[1];  

  return((int)r);
}

int extract_int32(unsigned char *p, int halfwordid){

  int r;	/* result */
  unsigned char *b = p;

  b += (halfwordid - 1) * 2;
  r = (b[0] << 24) + (b[1] << 16) + (b[2] << 8) + b[3];

  return(r);
}

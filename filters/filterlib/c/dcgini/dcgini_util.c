/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include "dcgini_util.h"

unsigned int dcgini_unpack_uint16(void *p, size_t start){
  /*
   * The first byte is the most significant one and the last byte is
   * the least significant.
   */
  unsigned int u;
  unsigned char *uptr;

  uptr = &((unsigned char*)p)[start];

  u = (uptr[0] << 8) + uptr[1];

  return(u);
}

unsigned int dcgini_unpack_uint24(void *p, size_t start){
  /*
   * The first byte is the most significant one and the last byte is
   * the least significant.
   */
  unsigned int u;
  unsigned char *uptr;

  uptr = &((unsigned char*)p)[start];

  u = (uptr[0] << 16) + (uptr[1] << 8) + uptr[2];

  return(u);
}

unsigned int dcgini_unpack_uint32(void *p, size_t start){
  /*
   * The first byte is the most significant one and the last byte is
   * the least significant.
   */
  unsigned int u;
  unsigned char *uptr;

  uptr = &((unsigned char*)p)[start];

  u = (uptr[0] << 24) + (uptr[1] << 16) + (uptr[2] << 8) + uptr[3];

  return(u);
}


int dcgini_unpack_int24(void *p, size_t start){
  /*
   * The first bit is the sign bit: it is set for West and South.
   */
  int i;
  unsigned int u;
  unsigned char *uptr;

  uptr = &((unsigned char*)p)[start];

  /* mask to pick up only the last seven bits of uptr[0] */
  u = ((uptr[0] & 127) << 16) + (uptr[1] << 8) + uptr[2];
  i = (int)u;

  if((uptr[0] & 128) != 0)
    i *= -1;

  return(i);
}

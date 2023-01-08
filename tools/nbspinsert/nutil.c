/*
 * Copyright (c) 2005-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include "nutil.h"

uint32_t unpack_uint32(void *p, size_t start){
  /*
   * The first byte is the most significant
   * one and the last byte is the least significant.
   */
  uint32_t u;
  unsigned char *uptr;

  uptr = &((unsigned char*)p)[start];

  u = (uptr[0] << 24) + (uptr[1] << 16) + (uptr[2] << 8) + uptr[3];

  return(u);
}

void pack_uint32(void *p, uint32_t u, size_t start){

  unsigned char *uptr;

  uptr = &((unsigned char*)p)[start];

  uptr[3] = u & 0xff;
  uptr[2] = (u >> 8) & 0xff;
  uptr[1] = (u >> 16) & 0xff;
  uptr[0] = (u >> 24) & 0xff;
}

uint16_t unpack_uint16(void *p, size_t start){
  /*
   * The first byte is the most significant
   * one and the last byte is the least significant.
   */
  uint16_t u;
  unsigned char *uptr;

  uptr = &((unsigned char*)p)[start];

  u = (uptr[0] << 8) + uptr[1];

  return(u);
}

void pack_uint16(void *p, uint16_t u, size_t start){

  unsigned char *uptr;

  uptr = &((unsigned char*)p)[start];

  uptr[1] = u & 0xff;
  uptr[0] = (u >> 8) & 0xff;
}

uint32_t calc_checksum(void *data, size_t size){

  size_t i;
  unsigned char *p;
  uint32_t cs;

  p = (unsigned char*)data;
  cs = 0;
  for(i = 0; i < size; ++i)
    cs += p[i];

  return(cs);
}

int valid_str(char *s){

  if((s != NULL) && (s[0] != '\0'))
    return(1);

  return(0);
}

char *trimleft(char *s, char *t){

  char *p = s;

  if(t == NULL){
    while(isspace(*p))
      ++p;
  } else {
    while((strchr(t, *p) != NULL) && (*p != '\0'))
      ++p;
  }

  if(*p == '\0')
    return(s);

  return(p);
}

char *trimright(char *s, char *t){
  /*
   * Note: This function modifies the "s" argument.
   */
  char *q;
  size_t size;

  size = strlen(s);
  if(size == 0)
    return(s);

  q = &s[size - 1];

  if(t == NULL){
    while(isspace(*q) && (q != s)){
      *q = '\0';
      --q;
    }
  } else {
    while((strchr(t, *q) != NULL) && (q != s)){
      *q = '\0';
      --q;
    }
  }

  return(s);
}

/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCGINI_UTIL_H
#define DCGINI_UTIL_H

#include <sys/types.h>

unsigned int dcgini_unpack_uint16(void *p, size_t start);
unsigned int dcgini_unpack_uint24(void *p, size_t start);
unsigned int dcgini_unpack_uint32(void *p, size_t start);
int dcgini_unpack_int24(void *p, size_t start);

#endif

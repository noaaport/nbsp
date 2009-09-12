/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef STOI_H
#define STOI_H

#include <stdint.h>

int strto_int(char *s, int *val);
int strto_uint(char *s, unsigned int *val);
int strto_double(char *s, double *val);
int strto_u16(char *s, uint16_t *val);
int strto_u32(char *s, uint32_t *val);

#endif

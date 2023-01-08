/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef NUTIL_H
#define NUTIL_H

#include <sys/types.h>
#include <stdint.h>

uint32_t unpack_uint32(void *p, size_t start);
void pack_uint32(void *p, uint32_t u, size_t start);
uint16_t unpack_uint16(void *p, size_t start);
void pack_uint16(void *p, uint16_t u, size_t start);
uint32_t calc_checksum(void *data, size_t size);

int valid_str(char *s);

char *trimleft(char *s, char *t);
char *trimright(char *s, char *t);

#endif

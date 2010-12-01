/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCNIDS_EXTRACT_H
#define DCNIDS_EXTRACT_H

int extract_uint8(unsigned char *p, int halfwordid);
int extract_uint16(unsigned char *p, int halfwordid);
uint32_t extract_uint32(unsigned char *p, int halfwordid);
int extract_int16(unsigned char *p, int halfwordid);
int extract_int32(unsigned char *p, int halfwordid);

#endif


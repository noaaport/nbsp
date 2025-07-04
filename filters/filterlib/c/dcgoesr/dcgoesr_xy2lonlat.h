/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

#ifndef DCGOESR_XY2LONLAT_H
#define DCGOESR_XY2LONLAT_H

/*
 * These definitions are here only for reference purposes.
 * They must be passed to the function, and they are actually
 * extracted from the nc file in the calling function.
 */
#define LON_ORIGIN_EAST -75.0
#define LON_ORIGIN_WEST -137.0

void xy2lonlat(float x, float y, float *lon, float *lat,
	       float lorigin);

void lonlat2xy(float lon, float lat, float *x, float *y, 
	       float lorigin);
#endif

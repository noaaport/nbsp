/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

#ifndef XY2LONLAT_H
#define XY2LONLAT_H

/*
 * These must be passed to the function.
 */
#define LON_ORIGIN_EAST -75.0
#define LON_ORIGIN_WEST -137.0

void xy2lonlat(double x, double y, double *lon, double *lat,
	       double lorigin);
#endif

/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCGINI_GRID_H
#define DCGINI_GRID_H

/*
 * Any given (lon, lat) point is reached by a pait of integers in the
 * form
 *
 * lon = lon1 + k * dlon
 * lat = lat1 + l * dlat
 *
 * where
 *
 * 0 <= k <= nlon - 1
 * 0 <= l <= nlat - 1
 *
 * Therefore
 *
 * lon2 = lon1 + (nlon - 1) * dlon ==> dlon = (lon2 - lon1)/(nlon - 1)
 *
 * and similarly for dlat.
 *
 * The level valus of a point (k, l) is the element
 *
 * level[l * nlon + k]
 *
 * For a given (lon, lat) coresponding to a given (k, l), we compute
 * the corresponding (x, y) using the projection transformations,
 * and then determine the associated (i, j) that is defined by
 * the formulas such as (for the ps projection)
 *
 * x = pstr->x1 + (double)i * pstr->dx;
 * y = pstr->y1 + pstr->s * (double)j * pstr->dy;
 *
 * and similarly for the other projections.
 *
 * Then we store in level[l * nlon + k], the value of datap[j * ny + i] if
 * the computed (i, j) fall within the original (nx, ny) bounds, or
 * DCGINI_GRID_MAP_OUTOFBOUNDS otherwise.
 */

/* level value for points that lie outside the original array bounds */
#define DCGINI_GRID_MAP_OUTOFBOUNDS 0

struct dcgini_grid_map_st {
  unsigned char *level;
  size_t numpoints;     /* nlon * nlat */
  size_t nlon;
  size_t nlat;
  double lon1_deg;
  double lat1_deg;
  double lon2_deg;
  double lat2_deg;
  double dlon_deg;
  double dlat_deg;
  double cellsize_deg;	/* set only for square "asc" cell grids */
};

#endif

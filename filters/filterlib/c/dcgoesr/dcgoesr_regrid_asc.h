/*
 * Copyright (c) 2025 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCGOESR_REGRID_ASC_H
#define DCGOESR_REGRID_ASC_H

#include "dcgoesr.h"

/*
 * level value for points that lie outside the original array bounds
 * in the regrid.
 */
#define DCGOESR_GRID_MAP_NODATA -1

int dcgoesr_regrid_data_asc(struct dcgoesr_point_map_st *pmap,
			    char *llur_str,
			    int f_llur_str_diff,
			    struct dcgoesr_point_map_st **gmap);
#endif

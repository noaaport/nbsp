/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCGINI_TRANSFORM_H
#define DCGINI_TRANSFORM_H

#include "dcgini.h"

int dcgini_transform_data(struct dcgini_st *dcg);
int dcgini_regrid_data(struct dcgini_st *dcg);
int dcgini_regrid_data_asc(struct dcgini_st *dcg,
			   char *llur_str,
			   int f_llur_str_diff);

#endif

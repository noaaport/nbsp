/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCGINI_SHP_H
#define DCGINI_SHP_H

#include <stdint.h>
#include "dcgini.h"

void dcgini_shp_insert_uint32_big(unsigned char *p, int pos, uint32_t u);
void dcgini_shp_insert_uint32_little(unsigned char *p, int pos, uint32_t u);
void dcgini_shp_insert_double_little(unsigned char *p, int pos, double r);

void dcgini_shp_insert_header(unsigned char *p,
			      int numpoints,
			      double lon_min, double lat_min,
			      double lon_max, double lat_max);
int dcgini_shp_point_record_size_words(void);
int dcgini_shp_point_file_size_words(int numpoints);
int dcgini_shp_header_record_size_bytes(void);
void dcgini_shp_insert_content(unsigned char *p,
                               struct dcgini_point_map_st *pm);

int dcgini_shx_point_file_size_words(int numpoints);
void dcgini_shx_insert_content(unsigned char *p,
                               struct dcgini_point_map_st *pm);

struct dcgini_shp_st *dcgini_shp_create(struct dcgini_point_map_st *pm);
void dcgini_shp_destroy(  struct dcgini_shp_st *dcgini_shp);

int dcgini_shp_write_data(int shp_fd, int shx_fd,
			  struct dcgini_point_map_st *pm);
#endif

/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <inttypes.h>
#include "dcnids.h"

#ifndef DCNIDS_SHP_H
#define DCNIDS_SHP_H

uint32_t dcnids_shp_extract_uint32_big(unsigned char *p, int pos);
uint32_t dcnids_shp_extract_uint32_little(unsigned char *p, int pos);
void dcnids_shp_insert_uint32_big(unsigned char *p, int pos, uint32_t u);
void dcnids_shp_insert_uint32_little(unsigned char *p, int pos, uint32_t u);
double dcnids_shp_extract_double_little(unsigned char *p, int pos);
void dcnids_shp_insert_double_little(unsigned char *p, int pos, double r);

int dcnids_shp_insert_polygon(unsigned char *p, int pos,
			      int record_number, double *lon, double *lat);
void dcnids_shp_insert_header(unsigned char *p,
			      int numpolygons,
			      double lon_min, double lat_min,
			      double lon_max, double lat_max);
int dcnids_shp_header_record_size_bytes(void);
int dcnids_shp_polygon_record_size_words(void);
int dcnids_shp_polygon_file_size_words(int numpolygons);
void dcnids_shp_insert_content(unsigned char *p,
			       struct dcnids_polygon_map_st *pm);

int dcnids_shx_polygon_file_size_words(int numpolygons);
void dcnids_shx_insert_content(unsigned char *p,
			       struct dcnids_polygon_map_st *pm);

struct dcnids_shp_st *dcnids_shp_create(struct dcnids_polygon_map_st *pm);
void dcnids_shp_destroy(struct dcnids_shp_st *dcnids_shp);

#endif

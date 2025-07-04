/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 * Copyright (c) 2025 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
/*
 * Private functions for dcgoesr_shp (this is a copy of the dcgini_shp)
 */
#ifndef DCGOESR_SHP_H
#define DCGOESR_SHP_H

#include <stdint.h>
#include "dcgoesr.h"

/* in-memory shapefile */
struct dcgoesr_shp_st {
  unsigned char *b;     /* buffer */
  uint32_t size;
  uint32_t shpsize;
  uint32_t shxsize;
};

void dcgoesr_shp_insert_uint32_big(unsigned char *p, int pos, uint32_t u);
void dcgoesr_shp_insert_uint32_little(unsigned char *p, int pos, uint32_t u);
void dcgoesr_shp_insert_float_little(unsigned char *p, int pos, float r);

void dcgoesr_shp_insert_header(unsigned char *p,
			      int numpoints,
			      float lon_min, float lat_min,
			      float lon_max, float lat_max);
int dcgoesr_shp_point_record_size_words(void);
int dcgoesr_shp_point_file_size_words(int numpoints);
int dcgoesr_shp_header_record_size_bytes(void);
void dcgoesr_shp_insert_content(unsigned char *p,
                               struct dcgoesr_point_map_st *pm);

int dcgoesr_shx_point_file_size_words(int numpoints);
void dcgoesr_shx_insert_content(unsigned char *p,
                               struct dcgoesr_point_map_st *pm);

struct dcgoesr_shp_st *dcgoesr_shp_create(struct dcgoesr_point_map_st *pm);
void dcgoesr_shp_destroy(struct dcgoesr_shp_st *dcgoesr_shp);

int dcgoesr_shp_write_data(int shp_fd, int shx_fd,
			  struct dcgoesr_point_map_st *pm);
#endif

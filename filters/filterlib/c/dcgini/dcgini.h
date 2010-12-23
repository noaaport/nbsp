/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCGINI_H
#define DCGINI_H

#include <inttypes.h>
#include "dcgini_pdb.h"

struct gini_data_st {
  unsigned char *data;    /* file data excluding wmo and pdb headers */
  size_t data_size;       /* numlines * linesize */
};

struct dcgini_point_st {
  double lon;
  double lat;
  int level;
};

struct dcgini_point_map_st {
  struct dcgini_point_st *points;
  size_t numpoints;
  double lon_min;
  double lat_min;
  double lon_max;
  double lat_max;
};

struct dcgini_shp_st {
  unsigned char *b;     /* buffer */
  uint32_t size;
  uint32_t shpsize;
  uint32_t shxsize;
};

struct dcgini_st {
  struct nesdis_pdb_st pdb;
  struct gini_data_st ginidata;
  struct dcgini_point_map_st pointmap;
  struct dcgini_shp_st shp;
};

int dcgini_shp_write(char *shpfile, char *shxfile,
		     struct dcgini_point_map_st *pm);
int dcgini_dbf_write(char *file, struct dcgini_point_map_st *pm);
int dcgini_info_write(char *file, struct dcgini_st *dcg);
int dcgini_csv_write(char *file, struct dcgini_point_map_st *pm);

#endif

/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>	/* memcpy */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include "dcnids.h"
#include "dcnids_shp.h"

#define SHAPEFILE_HEADER_SIZE_BYTES 100
#define SHAPEFILE_HEADER_SIZE_WORDS 50

/*
 * Private functions
 */
uint32_t dcnids_shp_extract_uint32_big(unsigned char *p, int pos){

  uint32_t r;   /* result */
  unsigned char *b = p;

  b += pos;
  r = (b[0] << 24) + (b[1] << 16) + (b[2] << 8) + b[3];

  return(r);
}

uint32_t dcnids_shp_extract_uint32_little(unsigned char *p, int pos){

  uint32_t r;   /* result */
  unsigned char *b = p;

  b += pos;
  r = (b[3] << 24) + (b[2] << 16) + (b[1] << 8) + b[0];

  return(r);
}

void dcnids_shp_insert_uint32_big(unsigned char *p, int pos, uint32_t u){

  unsigned char *b = p;

  b += pos;

  b[3] = u & 0xff;
  b[2] = (u >> 8) & 0xff;
  b[1] = (u >> 16) & 0xff;
  b[0] = (u >> 24) & 0xff;
}

void dcnids_shp_insert_uint32_little(unsigned char *p, int pos, uint32_t u){

  unsigned char *b = p;

  b += pos;

  b[0] = u & 0xff;
  b[1] = (u >> 8) & 0xff;
  b[2] = (u >> 16) & 0xff;
  b[3] = (u >> 24) & 0xff;
}

double dcnids_shp_extract_double_little(unsigned char *p, int pos){

  double r;   /* result */
  unsigned char *b = p;
  unsigned char *rp;
  int i;

  b += pos;
  rp = (unsigned char*)&r;

  for(i = 0; i < 8; ++i)
    rp[i] = b[i];

  return(r);
}

void dcnids_shp_insert_double_little(unsigned char *p, int pos, double r){

  unsigned char *b = p;
  unsigned char *rp;
  int i;

  b += pos;
  rp = (unsigned char*)&r;
  for(i = 0; i < 8; ++i)
    b[i] = rp[i];
}

int dcnids_shp_insert_polygon(unsigned char *p, int pos,
			      int record_number, double *lon, double *lat){
  /*
   * The lon and lat pointers contain the coordinates of the four points.
   *
   * This function inserts:
   *
   * the record number		4 bytes
   * the record length		4 bytes
   * the data of the record	128 bytes
   *
   * The 128 bytes of the data come from:
   *
   * 48 bytes up to the array of the numparts indices (which is only
   *    one item for us)
   *
   * 5 * 16 = 80 for the 5 points in the polygon
   *
   * The "record length", in 16 bit words, then is 128/2 = 64.
   *
   * Returns:
   *          The total number of 16 bit words consumed = (8 + 128)/2 = 68
   */
  unsigned char *b = p;
  double lon_min, lat_min, lon_max, lat_max;
  uint32_t shapetype, numparts, numpoints, partindex;
  uint32_t record_length;
  int i;

  shapetype = 5;
  numparts = 1;
  numpoints = 5;
  partindex = 0;
  record_length = 64;

  lon_min = 180.0;
  lat_min = 180.0;
  lon_max = -180.0;
  lat_max = -180.0;

  for(i = 0; i < 4; ++i){
    if(lon[i] < lon_min)
      lon_min = lon[i];

    if(lon[i] > lon_max)
      lon_max = lon[i];

    if(lat[i] < lat_min)
      lat_min = lat[i];

    if(lat[i] > lat_max)
      lat_max = lat[i];
  }

  b += pos;

  dcnids_shp_insert_uint32_big(b, 0, (uint32_t)record_number);
  b += 4;
  dcnids_shp_insert_uint32_big(b, 0, record_length);
  b += 4;
  
  dcnids_shp_insert_uint32_little(b, 0, shapetype);
  b += 4;

  dcnids_shp_insert_double_little(b, 0, lon_min);
  dcnids_shp_insert_double_little(b, 8, lat_min);
  dcnids_shp_insert_double_little(b, 16, lon_max);
  dcnids_shp_insert_double_little(b, 24, lat_max);
  b += 32;

  dcnids_shp_insert_uint32_little(b, 0, numparts);
  b += 4;
  
  dcnids_shp_insert_uint32_little(b, 0, numpoints);
  b += 4;

  dcnids_shp_insert_uint32_little(b, 0, partindex);
  b += 4;

  for(i = 0; i < 4; ++i){
    dcnids_shp_insert_double_little(b, 0, lon[i]);
    dcnids_shp_insert_double_little(b, 8, lat[i]);
    b += 16;
  }
  dcnids_shp_insert_double_little(b, 0, lon[0]);
  dcnids_shp_insert_double_little(b, 8, lat[0]);

  /*
   * Return the total number of 16 bit words consumed.
   */

  return(68);
}

void dcnids_shp_insert_header(unsigned char *p,
			      int numpolygons,
			      double lon_min, double lat_min,
			      double lon_max, double lat_max){
  unsigned char *b = p;
  uint32_t filecode;
  uint32_t filelength;
  uint32_t version;
  uint32_t shapetype;
  double unused;

  filecode = 9994;
  version = 1000;
  shapetype = 5;
  unused = 0.0;

  filelength = dcnids_shp_polygon_file_size_words(numpolygons);
  
  dcnids_shp_insert_uint32_big(b, 0, filecode);
  dcnids_shp_insert_uint32_big(b, 4, 0);
  dcnids_shp_insert_uint32_big(b, 8, 0);
  dcnids_shp_insert_uint32_big(b, 12, 0);
  dcnids_shp_insert_uint32_big(b, 16, 0);
  dcnids_shp_insert_uint32_big(b, 20, 0);

  dcnids_shp_insert_uint32_big(b, 24, filelength);
  dcnids_shp_insert_uint32_little(b, 28, version);
  dcnids_shp_insert_uint32_little(b, 32, shapetype);

  dcnids_shp_insert_double_little(b, 36, lon_min);
  dcnids_shp_insert_double_little(b, 44, lat_min);
  dcnids_shp_insert_double_little(b, 52, lon_max);
  dcnids_shp_insert_double_little(b, 60, lat_max);

  dcnids_shp_insert_double_little(b, 68, unused);
  dcnids_shp_insert_double_little(b, 76, unused);
  dcnids_shp_insert_double_little(b, 84, unused);
  dcnids_shp_insert_double_little(b, 92, unused);  
}

int dcnids_shp_polygon_record_size_words(void){
  /*
   * This is just a convenience definition to isolate this calculation
   * in one place. The result given here is actually documented in the
   * dcnids_shp_insert_polygon() function.
   *
   * The result is given in 16bit words.
   */

  return(68);
}

int dcnids_shp_polygon_file_size_words(int numpolygons){
  /*
   * The file size in 16bit words.
   */
  int r;
  int polygon_record_size;

  polygon_record_size = dcnids_shp_polygon_record_size_words();
  r = SHAPEFILE_HEADER_SIZE_WORDS + polygon_record_size * numpolygons;

  return(r);
}

int dcnids_shp_header_record_size_bytes(void){

  return(SHAPEFILE_HEADER_SIZE_BYTES);
}

void dcnids_shp_insert_content(unsigned char *p,
			       struct dcnids_polygon_map_st *pm){

  unsigned char *b = p;
  int record_size_bytes;
  int nwords;
  int i;
  int record_number;

  record_size_bytes = dcnids_shp_polygon_record_size_words() * 2;

  dcnids_shp_insert_header(b,
			   pm->numpolygons,
			   pm->lon_min,
			   pm->lat_min,
			   pm->lon_max,
			   pm->lat_max);

  b += SHAPEFILE_HEADER_SIZE_BYTES;
  /*
   * Record numbers are 1-based
   */
  record_number = 1;
  for(i = 0; i < pm->numpolygons; ++i){
    nwords = dcnids_shp_insert_polygon(b, 0, record_number,
		    pm->polygons[i].lon, pm->polygons[i].lat);

    /* assert(nwords*2 == record_size_bytes); */

    b += record_size_bytes;
    ++record_number;
  }
}

/* shx functions */
int dcnids_shx_polygon_file_size_words(int numpolygons){

  int r;

  r = SHAPEFILE_HEADER_SIZE_WORDS + numpolygons * 4;

  return(r);
}

void dcnids_shx_insert_content(unsigned char *p,
			       struct dcnids_polygon_map_st *pm){

  unsigned char *b = p;
  uint32_t filelength;
  uint32_t record_offset_words;
  uint32_t record_size_words;
  int i;

  filelength = (uint32_t)dcnids_shx_polygon_file_size_words(pm->numpolygons);

  /*
   * Copy the method for the shp file, but then correct the file size.
   */
  dcnids_shp_insert_header(b, pm->numpolygons,
			   pm->lon_min,
			   pm->lat_min,
			   pm->lon_max,
			   pm->lat_max);
  dcnids_shp_insert_uint32_big(b, 24, filelength);  
  b += SHAPEFILE_HEADER_SIZE_BYTES;

  record_size_words = (uint32_t)dcnids_shp_polygon_record_size_words();
  record_offset_words = SHAPEFILE_HEADER_SIZE_WORDS;
  for(i = 0; i < pm->numpolygons; ++i){
    dcnids_shp_insert_uint32_big(b, 0, record_offset_words);
    dcnids_shp_insert_uint32_big(b, 4, record_size_words);
    b += 8;
    record_offset_words += record_size_words;
  }
}

/*
 * Public functions (declared in dcnids.h)
 */

struct dcnids_shp_st *dcnids_shp_create(struct dcnids_polygon_map_st *pm){

  struct dcnids_shp_st *dcnids_shp;
  unsigned char *b;

  dcnids_shp = malloc(sizeof(struct dcnids_shp_st));
  if(dcnids_shp == NULL)
    return(NULL);

  dcnids_shp->b = NULL;

  dcnids_shp->shpsize = dcnids_shp_polygon_file_size_words(pm->numpolygons);
  dcnids_shp->shxsize = dcnids_shx_polygon_file_size_words(pm->numpolygons); 

  /* in bytes */
  dcnids_shp->shpsize *= 2;
  dcnids_shp->shxsize *= 2;
  dcnids_shp->size = dcnids_shp->shpsize + dcnids_shp->shxsize;
    
  dcnids_shp->b = malloc(dcnids_shp->size);
  if(dcnids_shp->b == NULL){
    free(dcnids_shp);
    return(NULL);
  }

  b = dcnids_shp->b;
  dcnids_shp_insert_content(b, pm);
  b += dcnids_shp->shpsize;
  dcnids_shx_insert_content(b, pm);

  return(dcnids_shp);
}

void dcnids_shp_destroy(struct dcnids_shp_st *dcnids_shp){

  if(dcnids_shp == NULL)
    return;

  if(dcnids_shp->b != NULL)
    free(dcnids_shp->b);

  free(dcnids_shp);
}

int dcnids_shp_write(int shp_fd, int shx_fd, struct dcnids_polygon_map_st *pm){

  int status = 0;
  unsigned char *b;
  struct dcnids_shp_st *dcnids_shp;
  
  dcnids_shp = dcnids_shp_create(pm);
  if(dcnids_shp == NULL)
    return(-1);

  b = dcnids_shp->b;
  if(write(shp_fd, b, dcnids_shp->shpsize) == -1){
    status = -1;
    goto End;
  }

  b += dcnids_shp->shpsize;
  if(write(shx_fd, b, dcnids_shp->shxsize) == -1){
    status = -1;
    goto End;
  }

 End:

  dcnids_shp_destroy(dcnids_shp);

  return(status);
}

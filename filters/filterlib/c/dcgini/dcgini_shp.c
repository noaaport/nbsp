/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
/* #include <stdio.h>	delete */
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "err.h"
#include "dcgini_shp.h"

#define SHAPEFILE_HEADER_SIZE_BYTES 100
#define SHAPEFILE_HEADER_SIZE_WORDS 50
#define SHAPETYPE_POINT	            1

/*
 * Private functions
 */

void dcgini_shp_insert_uint32_big(unsigned char *p, int pos, uint32_t u){

  unsigned char *b = p;

  b += pos;

  b[3] = u & 0xff;
  b[2] = (u >> 8) & 0xff;
  b[1] = (u >> 16) & 0xff;
  b[0] = (u >> 24) & 0xff;
}

void dcgini_shp_insert_uint32_little(unsigned char *p, int pos, uint32_t u){

  unsigned char *b = p;

  b += pos;

  b[0] = u & 0xff;
  b[1] = (u >> 8) & 0xff;
  b[2] = (u >> 16) & 0xff;
  b[3] = (u >> 24) & 0xff;
}

void dcgini_shp_insert_double_little(unsigned char *p, int pos, double r){

  unsigned char *b = p;
  unsigned char *rp;
  int i;

  b += pos;
  rp = (unsigned char*)&r;
  for(i = 0; i < 8; ++i)
    b[i] = rp[i];
}

int dcgini_shp_insert_point(unsigned char *p, int pos,
			    int record_number, double lon, double lat){
  /*
   * This function inserts:
   *
   * the record number		4 bytes
   * the record length		4 bytes
   * the data of the record	20 bytes
   *
   * The 20 bytes of the data come from:
   *
   * 4 bytes for the shapetype
   * 2 * 8 = 16 for the 2 coordinates
   *
   * The "record length", in 16 bit words, then is 20/2 = 10.
   *
   * Returns:
   *          The total number of 16 bit words consumed = (8 + 20)/2 = 14
   */
  unsigned char *b = p;
  uint32_t shapetype;
  uint32_t record_length;

  shapetype = SHAPETYPE_POINT;
  record_length = 10;

  b += pos;

  dcgini_shp_insert_uint32_big(b, 0, (uint32_t)record_number);
  b += 4;
  dcgini_shp_insert_uint32_big(b, 0, record_length);
  b += 4;  
  dcgini_shp_insert_uint32_little(b, 0, shapetype);
  b += 4;

  dcgini_shp_insert_double_little(b, 0, lon);
  dcgini_shp_insert_double_little(b, 8, lat);

  /*
   * Return the total number of 16 bit words consumed.
   */

  return(14);
}

void dcgini_shp_insert_header(unsigned char *p,
			      int numpoints,
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
  shapetype = SHAPETYPE_POINT;
  unused = 0.0;

  filelength = dcgini_shp_point_file_size_words(numpoints);

  dcgini_shp_insert_uint32_big(b, 0, filecode);
  dcgini_shp_insert_uint32_big(b, 4, 0);
  dcgini_shp_insert_uint32_big(b, 8, 0);
  dcgini_shp_insert_uint32_big(b, 12, 0);
  dcgini_shp_insert_uint32_big(b, 16, 0);
  dcgini_shp_insert_uint32_big(b, 20, 0);

  dcgini_shp_insert_uint32_big(b, 24, filelength);
  dcgini_shp_insert_uint32_little(b, 28, version);
  dcgini_shp_insert_uint32_little(b, 32, shapetype);

  dcgini_shp_insert_double_little(b, 36, lon_min);
  dcgini_shp_insert_double_little(b, 44, lat_min);
  dcgini_shp_insert_double_little(b, 52, lon_max);
  dcgini_shp_insert_double_little(b, 60, lat_max);

  dcgini_shp_insert_double_little(b, 68, unused);
  dcgini_shp_insert_double_little(b, 76, unused);
  dcgini_shp_insert_double_little(b, 84, unused);
  dcgini_shp_insert_double_little(b, 92, unused);  
}

int dcgini_shp_point_record_size_words(void){
  /*
   * This is just a convenience definition to isolate this calculation
   * in one place. The result given here is actually documented in the
   * dcnids_shp_insert_point() function.
   *
   * The result is given in 16bit words.
   */

  return(14);
}

int dcgini_shp_point_file_size_words(int numpoints){
  /*
   * The file size in 16bit words.
   */
  int r;
  int point_record_size;

  point_record_size = dcgini_shp_point_record_size_words();
  r = SHAPEFILE_HEADER_SIZE_WORDS + point_record_size * numpoints;

  return(r);
}

int dcgini_shp_header_record_size_bytes(void){

  return(SHAPEFILE_HEADER_SIZE_BYTES);
}

void dcgini_shp_insert_content(unsigned char *p,
			       struct dcgini_point_map_st *pm){

  unsigned char *b = p;
  int record_size_bytes;
  int nwords;
  size_t i;
  int record_number;

  record_size_bytes = dcgini_shp_point_record_size_words() * 2;

  dcgini_shp_insert_header(b,
			   pm->numpoints,
			   pm->lon_min,
			   pm->lat_min,
			   pm->lon_max,
			   pm->lat_max);

  b += SHAPEFILE_HEADER_SIZE_BYTES;
  /*
   * Record numbers are 1-based
   */
  record_number = 1;
  for(i = 0; i < pm->numpoints; ++i){
    nwords = dcgini_shp_insert_point(b, 0, record_number,
		    pm->points[i].lon, pm->points[i].lat);

    /* assert(nwords*2 == record_size_bytes); */

    b += record_size_bytes;
    ++record_number;
  }
}

/* shx functions */
int dcgini_shx_point_file_size_words(int numpoints){

  int r;

  r = SHAPEFILE_HEADER_SIZE_WORDS + numpoints * 4;

  return(r);
}

void dcgini_shx_insert_content(unsigned char *p,
			       struct dcgini_point_map_st *pm){

  unsigned char *b = p;
  uint32_t filelength;
  uint32_t record_offset_words;
  uint32_t record_size_words;
  size_t i;

  filelength = (uint32_t)dcgini_shx_point_file_size_words(pm->numpoints);

  /*
   * Copy the method for the shp file, but then correct the file size.
   */
  dcgini_shp_insert_header(b, pm->numpoints,
			   pm->lon_min,
			   pm->lat_min,
			   pm->lon_max,
			   pm->lat_max);
  dcgini_shp_insert_uint32_big(b, 24, filelength);  
  b += SHAPEFILE_HEADER_SIZE_BYTES;

  record_size_words = (uint32_t)dcgini_shp_point_record_size_words();
  record_offset_words = SHAPEFILE_HEADER_SIZE_WORDS;
  for(i = 0; i < pm->numpoints; ++i){
    dcgini_shp_insert_uint32_big(b, 0, record_offset_words);
    dcgini_shp_insert_uint32_big(b, 4, record_size_words);
    b += 8;
    record_offset_words += record_size_words;
  }
}

struct dcgini_shp_st *dcgini_shp_create(struct dcgini_point_map_st *pm){

  struct dcgini_shp_st *dcgini_shp;
  unsigned char *b;

  dcgini_shp = malloc(sizeof(struct dcgini_shp_st));
  if(dcgini_shp == NULL)
    return(NULL);

  dcgini_shp->b = NULL;

  dcgini_shp->shpsize = dcgini_shp_point_file_size_words(pm->numpoints);
  dcgini_shp->shxsize = dcgini_shx_point_file_size_words(pm->numpoints); 

  /* in bytes */
  dcgini_shp->shpsize *= 2;
  dcgini_shp->shxsize *= 2;
  dcgini_shp->size = dcgini_shp->shpsize + dcgini_shp->shxsize;
    
  dcgini_shp->b = malloc(dcgini_shp->size);
  if(dcgini_shp->b == NULL){
    free(dcgini_shp);
    return(NULL);
  }

  b = dcgini_shp->b;
  dcgini_shp_insert_content(b, pm);
  b += dcgini_shp->shpsize;
  dcgini_shx_insert_content(b, pm);

  return(dcgini_shp);
}

void dcgini_shp_destroy(struct dcgini_shp_st *dcgini_shp){

  if(dcgini_shp == NULL)
    return;

  if(dcgini_shp->b != NULL)
    free(dcgini_shp->b);

  free(dcgini_shp);
}

int dcgini_shp_write_data(int shp_fd, int shx_fd,
			  struct dcgini_point_map_st *pm){

  int status = 0;
  unsigned char *b;
  struct dcgini_shp_st *dcgini_shp;
  
  dcgini_shp = dcgini_shp_create(pm);
  if(dcgini_shp == NULL)
    return(-1);

  b = dcgini_shp->b;

  if(shp_fd != -1){
    if(write(shp_fd, b, dcgini_shp->shpsize) == -1){
      status = -1;
      goto End;
    }
  }

  b += dcgini_shp->shpsize;

  if(shx_fd != -1){
    if(write(shx_fd, b, dcgini_shp->shxsize) == -1){
      status = -1;
      goto End;
    }
  }

 End:

  dcgini_shp_destroy(dcgini_shp);

  return(status);
}

/*
 * Public (declared in dcgini.h)
 */

int dcgini_shp_write(char *shpfile, char *shxfile,
		     struct dcgini_point_map_st *pm){

  int status = 0;
  int shp_fd = -1, shx_fd = -1;

  if(shpfile != NULL){
    shp_fd = open(shpfile, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if(shp_fd == -1){
      log_err(1, "Cannot open %s", shpfile);
      return(-1);
    }
  }

  if(shxfile != NULL){
    shx_fd = open(shxfile, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if(shx_fd == -1){
      if(shp_fd != -1)
	close(shp_fd);

      log_err(1, "Cannot open %", shxfile);
      return(-1);
    }
  }

  status = dcgini_shp_write_data(shp_fd, shx_fd, pm);

  if(shp_fd != -1)
    (void)close(shp_fd);

  if(shx_fd != -1)
    (void)close(shx_fd);

  if(status != 0)
    log_err(1, "Could not write shp data");

  return(status);
}

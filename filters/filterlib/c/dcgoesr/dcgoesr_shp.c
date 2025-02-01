/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 * Copyright (c) 2025 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
/*
 * The code is a copy of the dcgini_shp.c
 */
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "err.h"
#include "dcgoesr.h"	 /* Public */
#include "dcgoesr_shp.h" /* Private to this file */

#define SHAPEFILE_HEADER_SIZE_BYTES 100
#define SHAPEFILE_HEADER_SIZE_WORDS 50
#define SHAPETYPE_POINT	            1

/*
 * Public (declared in dcgoesr.h)
 */
int dcgoesr_shp_write(char *shpfile, char *shxfile,
		     struct dcgoesr_point_map_st *pm){

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

  status = dcgoesr_shp_write_data(shp_fd, shx_fd, pm);

  if(shp_fd != -1)
    (void)close(shp_fd);

  if(shx_fd != -1)
    (void)close(shx_fd);

  if(status != 0)
    log_err(1, "Could not write shp data");

  return(status);
}

/*
 * Private functions of this file
 */
void dcgoesr_shp_insert_uint32_big(unsigned char *p, int pos, uint32_t u){

  unsigned char *b = p;

  b += pos;

  b[3] = u & 0xff;
  b[2] = (u >> 8) & 0xff;
  b[1] = (u >> 16) & 0xff;
  b[0] = (u >> 24) & 0xff;
}

void dcgoesr_shp_insert_uint32_little(unsigned char *p, int pos, uint32_t u){

  unsigned char *b = p;

  b += pos;

  b[0] = u & 0xff;
  b[1] = (u >> 8) & 0xff;
  b[2] = (u >> 16) & 0xff;
  b[3] = (u >> 24) & 0xff;
}

void dcgoesr_shp_insert_double_little(unsigned char *p, int pos, double r){

  unsigned char *b = p;
  unsigned char *rp;
  int i;

  b += pos;
  rp = (unsigned char*)&r;
  for(i = 0; i < 8; ++i)
    b[i] = rp[i];
}

int dcgoesr_shp_insert_point(unsigned char *p, int pos,
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

  dcgoesr_shp_insert_uint32_big(b, 0, (uint32_t)record_number);
  b += 4;
  dcgoesr_shp_insert_uint32_big(b, 0, record_length);
  b += 4;  
  dcgoesr_shp_insert_uint32_little(b, 0, shapetype);
  b += 4;

  dcgoesr_shp_insert_double_little(b, 0, lon);
  dcgoesr_shp_insert_double_little(b, 8, lat);

  /*
   * Return the total number of 16 bit words consumed.
   */

  return(14);
}

void dcgoesr_shp_insert_header(unsigned char *p,
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

  filelength = dcgoesr_shp_point_file_size_words(numpoints);

  dcgoesr_shp_insert_uint32_big(b, 0, filecode);
  dcgoesr_shp_insert_uint32_big(b, 4, 0);
  dcgoesr_shp_insert_uint32_big(b, 8, 0);
  dcgoesr_shp_insert_uint32_big(b, 12, 0);
  dcgoesr_shp_insert_uint32_big(b, 16, 0);
  dcgoesr_shp_insert_uint32_big(b, 20, 0);

  dcgoesr_shp_insert_uint32_big(b, 24, filelength);
  dcgoesr_shp_insert_uint32_little(b, 28, version);
  dcgoesr_shp_insert_uint32_little(b, 32, shapetype);

  dcgoesr_shp_insert_double_little(b, 36, lon_min);
  dcgoesr_shp_insert_double_little(b, 44, lat_min);
  dcgoesr_shp_insert_double_little(b, 52, lon_max);
  dcgoesr_shp_insert_double_little(b, 60, lat_max);

  dcgoesr_shp_insert_double_little(b, 68, unused);
  dcgoesr_shp_insert_double_little(b, 76, unused);
  dcgoesr_shp_insert_double_little(b, 84, unused);
  dcgoesr_shp_insert_double_little(b, 92, unused);  
}

int dcgoesr_shp_point_record_size_words(void){
  /*
   * This is just a convenience definition to isolate this calculation
   * in one place. The result given here is actually documented in the
   * dcnids_shp_insert_point() function.
   *
   * The result is given in 16bit words.
   */

  return(14);
}

int dcgoesr_shp_point_file_size_words(int numpoints){
  /*
   * The file size in 16bit words.
   */
  int r;
  int point_record_size;

  point_record_size = dcgoesr_shp_point_record_size_words();
  r = SHAPEFILE_HEADER_SIZE_WORDS + point_record_size * numpoints;

  return(r);
}

int dcgoesr_shp_header_record_size_bytes(void){

  return(SHAPEFILE_HEADER_SIZE_BYTES);
}

void dcgoesr_shp_insert_content(unsigned char *p,
			       struct dcgoesr_point_map_st *pm){

  unsigned char *b = p;
  int record_size_bytes;
  int nwords;
  size_t i;
  int record_number;

  record_size_bytes = dcgoesr_shp_point_record_size_words() * 2;

  dcgoesr_shp_insert_header(b,
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
    nwords = dcgoesr_shp_insert_point(b, 0, record_number,
		    pm->points[i].lon, pm->points[i].lat);

    assert(nwords*2 == record_size_bytes);

    b += record_size_bytes;
    ++record_number;
  }
}

/* shx functions */
int dcgoesr_shx_point_file_size_words(int numpoints){

  int r;

  r = SHAPEFILE_HEADER_SIZE_WORDS + numpoints * 4;

  return(r);
}

void dcgoesr_shx_insert_content(unsigned char *p,
			       struct dcgoesr_point_map_st *pm){

  unsigned char *b = p;
  uint32_t filelength;
  uint32_t record_offset_words;
  uint32_t record_size_words;
  size_t i;

  filelength = (uint32_t)dcgoesr_shx_point_file_size_words(pm->numpoints);

  /*
   * Copy the method for the shp file, but then correct the file size.
   */
  dcgoesr_shp_insert_header(b, pm->numpoints,
			   pm->lon_min,
			   pm->lat_min,
			   pm->lon_max,
			   pm->lat_max);
  dcgoesr_shp_insert_uint32_big(b, 24, filelength);  
  b += SHAPEFILE_HEADER_SIZE_BYTES;

  record_size_words = (uint32_t)dcgoesr_shp_point_record_size_words();
  record_offset_words = SHAPEFILE_HEADER_SIZE_WORDS;
  for(i = 0; i < pm->numpoints; ++i){
    dcgoesr_shp_insert_uint32_big(b, 0, record_offset_words);
    dcgoesr_shp_insert_uint32_big(b, 4, record_size_words);
    b += 8;
    record_offset_words += record_size_words;
  }
}

struct dcgoesr_shp_st *dcgoesr_shp_create(struct dcgoesr_point_map_st *pm){

  struct dcgoesr_shp_st *dcgoesr_shp;
  unsigned char *b;

  dcgoesr_shp = malloc(sizeof(struct dcgoesr_shp_st));
  if(dcgoesr_shp == NULL)
    return(NULL);

  dcgoesr_shp->b = NULL;

  dcgoesr_shp->shpsize = dcgoesr_shp_point_file_size_words(pm->numpoints);
  dcgoesr_shp->shxsize = dcgoesr_shx_point_file_size_words(pm->numpoints); 

  /* in bytes */
  dcgoesr_shp->shpsize *= 2;
  dcgoesr_shp->shxsize *= 2;
  dcgoesr_shp->size = dcgoesr_shp->shpsize + dcgoesr_shp->shxsize;
    
  dcgoesr_shp->b = malloc(dcgoesr_shp->size);
  if(dcgoesr_shp->b == NULL){
    free(dcgoesr_shp);
    return(NULL);
  }

  b = dcgoesr_shp->b;
  dcgoesr_shp_insert_content(b, pm);
  b += dcgoesr_shp->shpsize;
  dcgoesr_shx_insert_content(b, pm);

  return(dcgoesr_shp);
}

void dcgoesr_shp_destroy(struct dcgoesr_shp_st *dcgoesr_shp){

  if(dcgoesr_shp == NULL)
    return;

  if(dcgoesr_shp->b != NULL)
    free(dcgoesr_shp->b);

  free(dcgoesr_shp);
}

int dcgoesr_shp_write_data(int shp_fd, int shx_fd,
			  struct dcgoesr_point_map_st *pm){

  int status = 0;
  unsigned char *b;
  struct dcgoesr_shp_st *dcgoesr_shp;
  
  dcgoesr_shp = dcgoesr_shp_create(pm);
  if(dcgoesr_shp == NULL)
    return(-1);

  b = dcgoesr_shp->b;

  if(shp_fd != -1){
    if(write(shp_fd, b, dcgoesr_shp->shpsize) == -1){
      status = -1;
      goto End;
    }
  }

  b += dcgoesr_shp->shpsize;

  if(shx_fd != -1){
    if(write(shx_fd, b, dcgoesr_shp->shxsize) == -1){
      status = -1;
      goto End;
    }
  }

 End:

  dcgoesr_shp_destroy(dcgoesr_shp);

  return(status);
}

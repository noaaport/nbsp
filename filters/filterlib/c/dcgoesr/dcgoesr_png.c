/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdlib.h>
#include <inttypes.h>
#include <png.h>
#include "dcgoesr_png.h"

/* public function */
int write_data_png(FILE *fp, unsigned char *data,
		   int linesize, int numlines){
  
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  int bit_depth = 8;
  int color_type = PNG_COLOR_TYPE_GRAY;
  int interlace_type = PNG_INTERLACE_NONE;
  int compression_type = PNG_COMPRESSION_TYPE_DEFAULT;
  int filter_method = PNG_FILTER_TYPE_DEFAULT;
  int width = linesize;
  int height = numlines;
  unsigned char *row;
  int i;
  volatile int status = 0;   /*
			      * warning: variable 'status' might be
			      * clobbered by `longjmp'
			      */

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(png_ptr != NULL){
    info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == NULL){
      png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
      png_ptr = NULL;
    }
  }

  if(png_ptr == NULL){
    status = -1;
    goto end;
  }

  if(setjmp(png_jmpbuf(png_ptr))){
    status = 1;		/* error from png lib */
    goto end;
  }

  png_init_io(png_ptr, fp);
  png_set_IHDR(png_ptr, info_ptr, width, height,
	       bit_depth, color_type, interlace_type,
	       compression_type, filter_method);
  png_write_info(png_ptr, info_ptr);

  row = data;
  for(i = 1; i <= numlines; ++i){
    png_write_row(png_ptr, row);
    row += linesize;
  }

 end:

  if(status == 0)
    png_write_end(png_ptr, info_ptr);
  
  if(png_ptr != NULL)
    png_destroy_write_struct(&png_ptr, &info_ptr);

  return(status);
}

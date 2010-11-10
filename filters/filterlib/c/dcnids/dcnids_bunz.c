/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdlib.h>
#include <bzlib.h>
#include "dcnids.h"

#define DCNIDS_BUNZ_BUFFER_SIZE		262144  /* 1024 * 256 */

int dcnids_bunz(unsigned char **data, unsigned int *datasize, int *bzerror){
  /*
   * Returns:
   *
   * -1 => system error
   *  1 => libbz2 error
   *  0
   *
   * NOTE: *data must point to memory allocated with malloc.
   */
  int status;
  unsigned char *bunz_data = NULL;
  unsigned int bunz_data_size;

  bunz_data_size = DCNIDS_BUNZ_BUFFER_SIZE;
  do{
    bunz_data = malloc(bunz_data_size);
    if(bunz_data == NULL)
      return(-1);

    status = BZ2_bzBuffToBuffDecompress((char*)bunz_data, &bunz_data_size,
					(char*)*data, *datasize,
					0, 0);
    if(status == BZ_OK)
      break;

    free(bunz_data);
    bunz_data_size *= 2;
  }while(status == BZ_OUTBUFF_FULL);

  if(status != BZ_OK){
    *bzerror = status;
    return(1);
  }

  free(*data);
  *data = bunz_data;
  *datasize = bunz_data_size;

  return(0);
}

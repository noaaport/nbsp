/*
 * The functions to fill the data and the headers in each frame
 */
#include <string.h>
#include <stdio.h>
#include "sbnpack.h"

static void fill_flh_test(struct sbnpack_st *sbnpack);
static void fill_pdh_test(struct sbnpack_st *sbnpack);
static void fill_psh_test(struct sbnpack_st *sbnpack);
static void fill_ccb_test(struct sbnpack_st *sbnpack);

void fill_blockdata(struct sbnpack_st *sbnpack){

  int i;
  int nframes = sbnpack->nframes;
  int datablock_size;
  char *readp = sbnpack->sbnpack_file.readp; /* pointer to the file data */

  for(i = 0; i < nframes; ++i){
    datablock_size = sbnpack->sbnpack_frame[i].datablock_size;
    memcpy(sbnpack->sbnpack_frame[i].datablock, readp, datablock_size);
    readp += datablock_size;
  }
}

void fill_headers(struct sbnpack_st *sbnpack){

  fill_flh_test(sbnpack);
  fill_pdh_test(sbnpack);
  fill_psh_test(sbnpack);
  fill_ccb_test(sbnpack);
}

static void fill_flh_test(struct sbnpack_st *sbnpack){

  int nframes = sbnpack->nframes;
  int i;

  for(i = 0; i < nframes; ++i){
    memset(sbnpack->sbnpack_frame[i].flh, 'A', FRAME_LEVEL_HEADER_SIZE);
  }  
}

static void fill_pdh_test(struct sbnpack_st *sbnpack){

  int nframes = sbnpack->nframes;
  int i;

  for(i = 0; i < nframes; ++i){
    memset(sbnpack->sbnpack_frame[i].pdh, 'B', PRODUCT_DEF_HEADER_SIZE);
  }  
}

static void fill_psh_test(struct sbnpack_st *sbnpack){

  memset(sbnpack->sbnpack_frame[0].psh, 'C', PRODUCT_SPEC_HEADER_SIZE);
}

static void fill_ccb_test(struct sbnpack_st *sbnpack){

  memset(sbnpack->sbnpack_frame[0].ccb, 'D', CCB_SIZE);
}

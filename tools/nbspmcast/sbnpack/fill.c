/*
 * The functions to fill the data and the headers in each frame
 */
#include <string.h>
#include <stdio.h>
#include "sbnpack_private.h"
#include "sbn.h"

static void fill_flh_test(struct sbnpack_st *sbnpack);
static void fill_pdh_test(struct sbnpack_st *sbnpack);
static void fill_psh_test(struct sbnpack_st *sbnpack);

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
  /*
   * The fill_<flh,pdh,psh> are in sbn.c
   */
  int nframes = sbnpack->nframes;
  int i;
  uint32_t prod_seq_number = sbnpack->prod_seq_number;
  uint32_t sbn_seq_number = sbnpack->sbn_seq_number;
  int psh_type_flag = sbnpack->psh_type_flag;

  /* First frame */
  fill_flh(&(sbnpack->sbnpack_frame[0]), sbn_seq_number);
  fill_pdh(&(sbnpack->sbnpack_frame[0]), prod_seq_number);
  fill_psh(&(sbnpack->sbnpack_frame[0]), psh_type_flag);

  /* The rest do not have a psh */
  for(i = 1; i < nframes; ++i){
    fill_flh(&(sbnpack->sbnpack_frame[i]), ++sbn_seq_number);
    fill_pdh(&(sbnpack->sbnpack_frame[i]), prod_seq_number);
  }
}

void fill_headers_test(struct sbnpack_st *sbnpack){

  fill_flh_test(sbnpack);
  fill_pdh_test(sbnpack);
  fill_psh_test(sbnpack);
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

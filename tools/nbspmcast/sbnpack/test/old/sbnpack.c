/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 */
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "file.h"
#include "sbnpack.h"

int init_sbnpack_file(char *fname, struct sbnpack_file_st *sbnpack_file){
  /*
   * Load the file data into sbnpack_file_st.
   * The pointer to the data (in the sbnpack_file_st) is created.
   */
  char *p = NULL;
  int fsize;
  int status = 0;

  status = get_file_size(fname, &fsize);
  if(status != 0)
    return(status);

  p = malloc(fsize);
  if(p == NULL)
    return(-1);

  status = load_file(fname, fsize, p);
  if(status != 0){
    free(p);
    return(status);
  }
  
  sbnpack_file->data = p;  
  sbnpack_file->allocated_size = fsize;
  sbnpack_file->data_size = fsize;
  sbnpack_file->readp = sbnpack_file->data;
  
  return(0);
}

int reinit_sbnpack_file(char *fname, struct sbnpack_file_st *sbnpack_file){
  /*
   * Load the file data into  the sbnpack_file_st.
   * The pointer to the data (in the sbnpack_file_st) is reused
   * if the allocated space is larger than the file size.
   */
  char *p = NULL;
  int fsize;
  int status = 0;

  status = get_file_size(fname, &fsize);
  if(status != 0)
    return(status);

  if(sbnpack_file->allocated_size < fsize){
    free(sbnpack_file->data);  
    p = malloc(fsize);
    if(p == NULL)
      return(-1);

    sbnpack_file->data = p;  
    sbnpack_file->allocated_size = fsize;
  }

  status = load_file(fname, fsize, sbnpack_file->data);
  if(status != 0)
    return(status);
  
  sbnpack_file->data_size = fsize;
  sbnpack_file->readp = sbnpack_file->data;
  
  return(0);
}

void end_sbnpack_file(struct sbnpack_file_st *sbnpack_file){

  if(sbnpack_file->data == NULL)
    return;

  free(sbnpack_file->data);
  sbnpack_file->data = NULL;
}

struct sbnpack_frame_st *create_sbnpack_frame_array(int nframes,
						    int last_datablock_size){
  /*
   * Create the array of frames, with the header pointers pointing
   * to the correct spot in each frame, but does not load any
   * data.
   */
  struct sbnpack_frame_st *sbnpack_frame_array;
  int i;
  int offset;

  fprintf(stdout, "A: nframes=%d, last_datablock_size=%d\n",
	  nframes, last_datablock_size);

  sbnpack_frame_array = malloc(nframes * sizeof(struct sbnpack_frame_st));
  if(sbnpack_frame_array == NULL)
    return(NULL);

  for(i = 0; i < nframes; ++i){
    sbnpack_frame_array[i].frame_index = i;
    sbnpack_frame_array[i].nframes = nframes;
  }

  offset = 0;
  
  /* first frame */
  sbnpack_frame_array[0].flh = &(sbnpack_frame_array[0].frame[0]);

  offset += FRAME_LEVEL_HEADER_SIZE;
  sbnpack_frame_array[0].pdh =
    &(sbnpack_frame_array[0].frame[offset]);

  offset += PRODUCT_DEF_HEADER_SIZE;
  sbnpack_frame_array[0].psh =
    &(sbnpack_frame_array[0].frame[offset]);

  offset += PRODUCT_SPEC_HEADER_SIZE;
  sbnpack_frame_array[0].ccb =
    &(sbnpack_frame_array[0].frame[offset]);

  offset += CCB_SIZE;
  sbnpack_frame_array[0].datablock =
        &(sbnpack_frame_array[0].frame[offset]);

  assert(offset == HEADER_SIZE_0);

  sbnpack_frame_array[0].datablock_size = DATA_BLOCK_SIZE;
  sbnpack_frame_array[0].header_size = HEADER_SIZE_0;
  sbnpack_frame_array[0].frame_index = 0;
  sbnpack_frame_array[0].nframes = nframes;

  for(i = 1; i < nframes; ++i) {
    offset = 0;
    
    sbnpack_frame_array[i].flh = &(sbnpack_frame_array[i].frame[0]);

    offset += FRAME_LEVEL_HEADER_SIZE;
    sbnpack_frame_array[i].pdh =
      &(sbnpack_frame_array[i].frame[offset]);

    offset += PRODUCT_DEF_HEADER_SIZE;

    /*
     * Only the first frame has a psh and ccb - so we do not
     * add them here.
     */

    sbnpack_frame_array[i].datablock =
        &(sbnpack_frame_array[i].frame[offset]);

    assert(offset == HEADER_SIZE);
    
    sbnpack_frame_array[i].datablock_size = DATA_BLOCK_SIZE;
    sbnpack_frame_array[i].header_size = HEADER_SIZE;
    sbnpack_frame_array[i].frame_index = i;
    sbnpack_frame_array[i].nframes = nframes;
  }

  /* The last frame */
  sbnpack_frame_array[nframes - 1].datablock_size = last_datablock_size;

  fprintf(stdout, "B: nframes=%d, last_datablock_size=%d\n",
	  sbnpack_frame_array[nframes - 1].nframes,
	  sbnpack_frame_array[nframes - 1].datablock_size);
  
  return(sbnpack_frame_array);
}

void destroy_sbnpack_frame_array(struct sbnpack_frame_st *sbnpack_frame){

  if(sbnpack_frame != NULL)
    free(sbnpack_frame);
}
  
int init_sbnpack(char *fname, struct sbnpack_st *sbnpack){

  int nframes;
  int last_datablock_size;
  int status = 0;

  sbnpack->sbnpack_file.data = NULL;
  sbnpack->sbnpack_frame = NULL;
  
  status = init_sbnpack_file(fname, &(sbnpack->sbnpack_file));

  if(status == 0)
    status = get_file_frame_params(DATA_BLOCK_SIZE,
				   sbnpack->sbnpack_file.data_size,
				   &nframes, &last_datablock_size);

  if(status == 0)
    sbnpack->nframes = nframes;

  fprintf(stdout, "C: nframes=%d, last_datablock_size=%d\n",
	  nframes, last_datablock_size);
  
  if(status == 0){
    sbnpack->sbnpack_frame =
      create_sbnpack_frame_array(nframes, last_datablock_size);
    
    if(sbnpack->sbnpack_frame == NULL)
      status = -1;
  }
  
  fprintf(stdout, "%s\n", "D");
  
  if(status == 0){
    fill_blockdata(sbnpack);
    fill_headers(sbnpack);
  }

  return(status);
}

void end_sbnpack(struct sbnpack_st *sbnpack){

  destroy_sbnpack_frame_array(sbnpack->sbnpack_frame);
  end_sbnpack_file(&(sbnpack->sbnpack_file));

  sbnpack->sbnpack_file.data = NULL;
  sbnpack->sbnpack_frame = NULL;
}

int send_sbnpack_frame(int fd, struct sbnpack_st *sbnpack, int findex){

  int framesize;
  int n;
  int status = 0;

  assert(findex < sbnpack->nframes);

  framesize = sbnpack->sbnpack_frame[findex].datablock_size +
    sbnpack->sbnpack_frame[findex].header_size;
  
  n = write(fd, sbnpack->sbnpack_frame[findex].frame, framesize);
  if(n == -1)
    status = -1;
  else if(n != framesize)
    status = 1;
  
  return(status);
}

int send_sbnpack(int fd, struct sbnpack_st *sbnpack){

  int nframes;
  int i;
  int status = 0;
  
  nframes = sbnpack->nframes;
  
  for(i = 0; i < nframes; ++i){
    status = send_sbnpack_frame(fd, sbnpack, i);
    if(status != 0)
      break;
  }

  return(status);
}

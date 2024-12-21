/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 */
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
/* #include <stdio.h> */
#include "sbnpack_private.h"
#include "file.h"

int init_sbnpack_file(char *fname, struct sbnpack_file_st *sbnpack_file){
  /*
   * Load the file data into the sbnpack_file_st.
   * The pointer to the data (in the sbnpack_file_st) is reused
   * if the allocated space is larger than the file size.
   * Otherwise a new storage is allocated, in particular if the
   * allocated size is zero in which case the function behaves as
   * the original init_sbnpack_file() above.
   * (We are not using this function yet, until we implement reading
   * more then one file either from the cmd line or from stdin.
   * Eventually this should be renamed init_sbnpack_file() and substitute
   * the one above.
   */
  char *p = NULL;
  int fsize;
  int status = 0;

  status = get_file_size(fname, &fsize);
  if(status != 0)
    return(status);
  
  if(sbnpack_file->allocated_size >= fsize){
    /** fprintf(stdout, "A: %s\n", fname); **/

    /* the old sbnpack_file is good enough */
    status = load_file(fname, fsize, sbnpack_file->data);
    if(status == 0) {
      sbnpack_file->data_size = fsize;
      sbnpack_file->readp = sbnpack_file->data;
    }

    return(status);
  }

  /** fprintf(stdout, "B: %s\n", fname); **/

  p = malloc(fsize);
  if(p == NULL)
    return(-1);
  
  status = load_file(fname, fsize, p);
  if(status != 0) {
    free(p);
    return(status);
  }
  
  if(sbnpack_file->data != NULL) {
    /** fprintf(stdout, "C: %s\n", fname); **/    
    free(sbnpack_file->data);
  }
    
  sbnpack_file->data = p;  
  sbnpack_file->allocated_size = fsize;
  sbnpack_file->data_size = fsize;
  sbnpack_file->readp = sbnpack_file->data;

  return(status);
}

void free_sbnpack_file(struct sbnpack_file_st *sbnpack_file){

  if(sbnpack_file->data == NULL)
    return;

  free(sbnpack_file->data);
  sbnpack_file->data = NULL;
  sbnpack_file->allocated_size = 0;
}

int init_sbnpack_frame_array(struct sbnpack_st *sbnpack,
			     int nframes,
			     int last_datablock_size){
  /*
   * Create the array of frames, with the header pointers pointing
   * to the correct spot in each frame, but does not load any
   * data.
   */
  struct sbnpack_frame_st *sbnpack_frame_array = NULL;
  int i;
  int offset;

  /*
  fprintf(stdout, "A: nframes=%d, last_datablock_size=%d\n",
	  nframes, last_datablock_size);
  */
  
  if(sbnpack->allocated_frames < nframes) {
    /** fprintf(stdout, "X\n"); **/

    sbnpack_frame_array = malloc(nframes * sizeof(struct sbnpack_frame_st));
    if(sbnpack_frame_array == NULL)
      return(-1);

    if(sbnpack->sbnpack_frame != NULL)
      free(sbnpack->sbnpack_frame);
    
    sbnpack->sbnpack_frame = sbnpack_frame_array;
    sbnpack->allocated_frames = nframes;
    sbnpack->nframes = nframes;
  } else {
    /** fprintf(stdout, "Y\n"); **/
    sbnpack_frame_array = sbnpack->sbnpack_frame;
    sbnpack->nframes = nframes;
  }

  /* This is common for all frames */
  for(i = 0; i < nframes; ++i){
    sbnpack_frame_array[i].frame_index = i;
    sbnpack_frame_array[i].nframes = nframes;
  }
  
  /* first frame */
  offset = 0;
  sbnpack_frame_array[0].flh = &(sbnpack_frame_array[0].frame[offset]);

  offset += FRAME_LEVEL_HEADER_SIZE;
  sbnpack_frame_array[0].pdh = &(sbnpack_frame_array[0].frame[offset]);

  offset += PRODUCT_DEF_HEADER_SIZE;
  sbnpack_frame_array[0].psh = &(sbnpack_frame_array[0].frame[offset]);

  offset += PRODUCT_SPEC_HEADER_SIZE;
  sbnpack_frame_array[0].datablock = &(sbnpack_frame_array[0].frame[offset]);

  assert(offset == HEADER_SIZE_0);

  sbnpack_frame_array[0].datablock_size = DATA_BLOCK_SIZE;
  sbnpack_frame_array[0].header_size = HEADER_SIZE_0;

  for(i = 1; i < nframes; ++i) {
    offset = 0;
    sbnpack_frame_array[i].flh = &(sbnpack_frame_array[i].frame[offset]);

    offset += FRAME_LEVEL_HEADER_SIZE;
    sbnpack_frame_array[i].pdh = &(sbnpack_frame_array[i].frame[offset]);

    offset += PRODUCT_DEF_HEADER_SIZE;

    /*
     * Only the first frame has a psh and ccb - so we do not
     * add them here.
     */

    sbnpack_frame_array[i].datablock = &(sbnpack_frame_array[i].frame[offset]);

    assert(offset == HEADER_SIZE);
    
    sbnpack_frame_array[i].datablock_size = DATA_BLOCK_SIZE;
    sbnpack_frame_array[i].header_size = HEADER_SIZE;
  }

  /* The last frame */
  sbnpack_frame_array[nframes - 1].datablock_size = last_datablock_size;

  /*
  fprintf(stdout, "B: last_datablock_size=%d\n",
	  sbnpack_frame_array[nframes - 1].datablock_size);
  */
  
  return(0);
}

void free_sbnpack_frame_array(struct sbnpack_st *sbnpack) {

  if(sbnpack == NULL)
    return;
  
  if(sbnpack->sbnpack_frame == NULL)
    return;
  
  free(sbnpack->sbnpack_frame);
  sbnpack->sbnpack_frame = NULL;
  sbnpack->allocated_frames = 0;
}

struct sbnpack_st *create_sbnpack(void) {
  /*
   * Create the (empty) sbnpack struct. It is to be filled (initialized)
   * for each file that will be processed, with init_sbnpack().
   */
  struct sbnpack_st *sbnp = NULL;
  
  sbnp = malloc(sizeof(struct sbnpack_st));
  if(sbnp == NULL)
    return(NULL);
  
  /* Initialize the relevant elements */
  sbnp->sbnpack_file.data = NULL;
  sbnp->sbnpack_file.allocated_size = 0;	/* allocated size */  
  sbnp->sbnpack_frame = NULL;
  sbnp->allocated_frames = 0;

  return(sbnp);
}

int init_sbnpack(struct sbnpack_st *sbnpack,
		 char *fname,
		 uint32_t prod_seq_number,
		 uint32_t sbn_seq_number,
		 int psh_type_flag){
  int nframes;
  int last_datablock_size;
  int status = 0;

  sbnpack->prod_seq_number = prod_seq_number;
  sbnpack->sbn_seq_number = sbn_seq_number;
  sbnpack->psh_type_flag = psh_type_flag;
    
  status = init_sbnpack_file(fname, &(sbnpack->sbnpack_file));

  if(status == 0)
    status = get_file_frame_params(DATA_BLOCK_SIZE,
				   sbnpack->sbnpack_file.data_size,
				   &nframes, &last_datablock_size);

  /*
  if(status == 0)
    fprintf(stdout, "C: nframes=%d, last_datablock_size=%d\n",
	    nframes, last_datablock_size);
  */
  
  if(status == 0)
    status = init_sbnpack_frame_array(sbnpack, nframes, last_datablock_size);
  
  if(status == 0){
    fill_blockdata(sbnpack);
    fill_headers(sbnpack);
  } else {
    /*
     * free the pointer to the file and reinitialize the file-related elements
     * and the same for the frames array, but do not free the pointer
     * to the sbnpack structure itself.
     */
    free_sbnpack(sbnpack);
  }

  return(status);
}

void free_sbnpack(struct sbnpack_st *sbnpack){
  /*
   * This function frees the contents of the structure, but does not free
   * the pointer itself. That must be done by the caller.
   * In the nbspmcast this is done by the cleanup function.
   */
  if(sbnpack == NULL)
    return;

  free_sbnpack_file(&(sbnpack->sbnpack_file));
  free_sbnpack_frame_array(sbnpack);
}

int write_sbnpack_frame(int fd, struct sbnpack_st *sbnpack, int findex){

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

int write_sbnpack(int fd, struct sbnpack_st *sbnpack){

  int nframes;
  int i;
  int status = 0;
  
  nframes = sbnpack->nframes;
  
  for(i = 0; i < nframes; ++i){
    status = write_sbnpack_frame(fd, sbnpack, i);
    if(status != 0)
      break;
  }

  return(status);
}

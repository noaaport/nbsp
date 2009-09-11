/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mfile.h"

/*
 * The absolute minimum size of the static pool for the memory files, and
 * the factor by which it grows when required.
 */
#define MF_STATIC_POOL_SIZE_MIN		8192
#define GROW_FACTOR			2

struct memframe_info_st {
  unsigned char f_compressed;
  size_t size;
};

static int grow_memfile(struct memfile_st *mf);

struct memfile_st *create_memfile(size_t mpoolsize){

  struct memfile_st *mf = NULL;
  size_t allocsize = MF_STATIC_POOL_SIZE_MIN;

  if(mpoolsize > allocsize)
    allocsize = mpoolsize;

  mf = malloc(sizeof(struct memfile_st));
  if(mf == NULL)
    return(NULL);

  mf->p = malloc(allocsize);
  if(mf->p == NULL){
    free(mf);
    return(NULL);
  }

  mf->allocsize = allocsize;
  mf->maxallocsize = allocsize;
  mf->size = 0;
  mf->nread = 0;

  return(mf);
}

void destroy_memfile(struct memfile_st *mf){

  if(mf == NULL)
    return;

  if(mf->p != NULL)
    free(mf->p);

  free(mf);
}

int open_memfile(struct memfile_st *mf){

  if((mf->size != 0) || (mf->nread != 0))
    return(1);

  return(0);
}

void close_memfile(struct memfile_st *mf){
  /*
   * Reinitalize the reading and writing pointers.
   */
  mf->size = 0;
  mf->nread = 0;
}

int realloc_memfile(struct memfile_st *mf, size_t newsize){

  char *p;

  p = realloc(mf->p, newsize);
  if(p == NULL)
    return(-1);

  mf->p = p;
  mf->allocsize = newsize;
  if(mf->allocsize > mf->maxallocsize)
    mf->maxallocsize = mf->allocsize;

  if(mf->size >= mf->allocsize)
    mf->size = mf->allocsize - 1;

  if(mf->nread >= mf->allocsize)
    mf->nread = mf->allocsize - 1;

  return(0);
}

static int grow_memfile(struct memfile_st *mf){

  int status = 0;
  size_t oldsize;
  size_t newsize;

  oldsize = mf->allocsize;
  newsize = GROW_FACTOR * oldsize;

  status = realloc_memfile(mf, newsize);

  return(status);
}

size_t get_memfile_allocated_size(struct memfile_st *mf){

  return(mf->allocsize);
}

size_t get_memfile_data_size(struct memfile_st *mf){

  return(mf->size);
}

int write_memfile(struct memfile_st *mf, void *data, int data_size){

  int status = 0;
  char *pnext;

  while((mf->size + data_size > mf->allocsize) && (status == 0))
    status = grow_memfile(mf);

  if(status != 0)
    return(status);

  pnext = mf->p;
  pnext += mf->size;
  memcpy(pnext, data, data_size);
  mf->size += data_size;

  return(status);
}

int write_memfile_fixed(struct memfile_st *mf, void *data, int data_size){
  /*
   * This is for fixed size memfile, that we do not allow to grow.
   * If the data does not fit, the function returns 1, otherwise it calls
   * write_memfile(), which should return 0.
   */ 
  int status = 0;

  if(mf->size + data_size > mf->allocsize)
    status = 1;
  else
    status = write_memfile(mf, data, data_size);

  return(status);
}

int save_memfile(int fd, struct memfile_st *mf){

  ssize_t n;
  char *p;
  int status = 0;

  p = mf->p;
   
  n = write(fd, p, mf->size);
  if((n < 0) || ((size_t)n != mf->size))
    status = -1;

  return(status);
}

int save_memfile_skip(int fd, struct memfile_st *mf, size_t skip){

  ssize_t n;
  char *p;
  size_t size;
  int status = 0;

  if(mf->size <= skip){
    return(1);
  }

  p = mf->p;
  p += skip;
  size = mf->size - skip;
   
  n = write(fd, p, size);
  if((n < 0) || ((size_t)n != size))
    status = -1;
  
  return(status);
}

int read_memfile(struct memfile_st *mf, void **pdata, int data_size){
  /*
   * Sets "pdata" to point to the current reading position. 
   * If there are data_size bytes or more left to be read,
   * returns data_size, otherwise whatever is left. Advances 
   * the internal reading pointer by amount of bytes returned.
   */
  char *rp;
  int rsize = data_size;

  rp = mf->p;
  
  if(mf->nread + data_size > mf->size)
    rsize = mf->size - mf->nread;

  rp += mf->nread;
  mf->nread += rsize;

  if(rsize > 0)
    *pdata = rp;

  return(rsize);
}

int write_memframe(struct memfile_st *mf, 
		  char *frdata, int frdata_size, int f_compressed){
  int n;

  n = write_memframe_info(mf, frdata_size, f_compressed);
  if(n != -1)
    n = write_memframe_data(mf, frdata, frdata_size);

  return(n);
};
  
int write_memframe_info(struct memfile_st *mf, 
		       int total_size, int f_compressed){
  /*
   * This function, together with write_memframe_data() can be used
   * separately instead of the above. Here the info struct is saved first.
   * Then write_memframe_data() can be called repeatedly to append 
   * frame data to the frame.
   */
  struct memframe_info_st mframeinfo;
  int n;

  mframeinfo.f_compressed = (unsigned char)f_compressed;
  mframeinfo.size = total_size;

  n = write_memfile(mf, &mframeinfo, sizeof(mframeinfo));

  return(n);
}

int write_memframe_data(struct memfile_st *mf, char *data, int data_size){

  int n;
  
  n = write_memfile(mf, data, data_size);

  return(n);
}

int get_memframe(struct memfile_st *mf, 
		 char **frdata, int *f_compressed){
  /*
   * Sets frdata to point to the start of the next frame found, and returns
   * the size (in bytes) of the frame, 0 if there are no frames left or -1
   * if there is an error.
   */
  struct memframe_info_st *mframeinfo;
  int n;
  int frdata_size;
  void *p = NULL;	/* debian-5.0 warned about uninitalized variable */

  n = read_memfile(mf, (void**)&mframeinfo, sizeof(struct memframe_info_st));
  if(n != 0){
    n = mframeinfo->size;
    *f_compressed = mframeinfo->f_compressed;
    frdata_size = read_memfile(mf, &p, n);
    if(frdata_size == n)
      *frdata = p;
    else{
      n = -1;	/* error */
    }
  }

  return(n);
}

int adjust_memfile_size(struct memfile_st *mf, size_t estimated_size){
  /*
   * Some memory is freed only if the currently allocated size
   * is the at the maximum.
   */
  int status = 0;
  size_t oldsize;
  size_t newsize = 0;

  oldsize = mf->allocsize;

  if(estimated_size > oldsize)
    newsize = estimated_size;
  else if((estimated_size * GROW_FACTOR < oldsize) &&
	  (oldsize == mf->maxallocsize)){
    newsize = oldsize/GROW_FACTOR;
  }

  if(newsize != 0)
    status = realloc_memfile(mf, newsize);
  
  return(status);
}

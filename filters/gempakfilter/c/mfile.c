/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mfile.h"

#ifdef TEST
#include <stdio.h>
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#define GROW_FACTOR 2

struct memframe_info_st {
  unsigned char f_compressed;
  int size;
};

static int grow_mf(struct memfile_st *mf);

struct memfile_st *open_memfile(int numfragments, int blocksize){

  char *p = NULL;
  struct memfile_st *mf = NULL;
  int size;
  
  size = numfragments * blocksize;
  p = malloc(size);
  if(p != NULL)
    mf = malloc(sizeof(struct memfile_st));

  if(mf == NULL){
    if(p != NULL)
      free(p);

    return(NULL);
  }

  mf->p = p;
  mf->maxsize = size;
  mf->size = 0;
  mf->nread = 0;

  return(mf);
}

void close_memfile(struct memfile_st *mf){

  if(mf == NULL)
    return;

  if(mf->p != NULL)
    free(mf->p);

  free(mf);
}

static int grow_mf(struct memfile_st *mf){

  char *p;
  int oldsize;
  int newsize;

#ifdef TEST
  fprintf(stdout, "growing: maxsize = %d, size = %d\n", mf->maxsize, mf->size);
#endif
 
  oldsize = mf->maxsize;
  newsize = GROW_FACTOR * oldsize;

  p = realloc(mf->p, newsize);
  if(p == NULL)
    return(-1);

  mf->p = p;
  mf->maxsize = newsize;

  return(0);
}

int write_memfile(struct memfile_st *mf, void *data, int data_size){

  int status = 0;
  char *pnext;

  while((mf->size + data_size > mf->maxsize) && (status == 0))
    status = grow_mf(mf);

  if(status != 0)
    return(status);

  pnext = mf->p;
  pnext += mf->size;
  memcpy(pnext, data, data_size);
  mf->size += data_size;

  return(status);
}

int save_memfile(int fd, struct memfile_st *mf){

  int n;

  n = write(fd, mf->p, mf->size);
  
  return(n);
}

int read_memfile(struct memfile_st *mf, void **pdata, int data_size){
  /*
   * Sets "pdata" to point to the current reading position. 
   * If there are data_size bytes or more left to be read,
   * returns datasize, otherwise whatever is left. Advances 
   * the internal reading pointer by amount of bytes returned.
   */
  char *rp;
  int rsize = data_size;
  
  if(mf->nread + data_size > mf->size)
    rsize = mf->size - mf->nread;

  rp = mf->p;
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
      n = 0;	/* error */
    }
  }

  return(n);
}

#ifdef TEST
int main(int argc, char **argv){

  int status = 0;
  struct memfile_st *mf;
  int fd;
  int i;
  char buffer[1024];
  int bufsize = 1024;
  int n;

  if(argc == 1)
    errx(1, "argc = 1");

  mf = open_memfile(argc - 2, 512);
  if(mf == NULL)
    err(1, "open_memfile()");

  for(i = 1; i <= argc - 1; ++i){
    fprintf(stdout, "working on %s\n", argv[i]);

    fd = open(argv[i], O_RDONLY);
    if(fd == -1)
      err(1, "open()");

    while((n = read(fd, buffer, bufsize)) > 0){
      status = write_memfile(mf, buffer, n);
      if(status == -1)
	err(1, "write_memfile()");
    }
    
    close(fd);
  }

  fd = open("mfile.test", O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if(fd == -1)
    err(1, "open(mfile.test)");

  n = save_memfile(fd, mf);
  if(n != mf->size)
    err(1, "save_memfile()");

  close(fd);

  close_memfile(mf);

  return(0);
}

#endif    
    

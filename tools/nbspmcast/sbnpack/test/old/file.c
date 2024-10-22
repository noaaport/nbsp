/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 */
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
/* #include <stdio.h> */
#include "file.h"

int get_file_size(char *fname, int *fsize){

  struct stat sb;
  off_t ofsize;
  int status = 0;
  
  status = stat(fname, &sb);
  if(status != 0)
    return(-1);
  
  ofsize = sb.st_size;
  if(ofsize > INT_MAX)
    return(1);

  *fsize = (int)ofsize;

  return(0);
}

int get_file_frame_params(int blocksize, int fsize,
			 int *nframes_ptr,
			 int *last_datablock_size_ptr){

  int nframes, last_datablock_size;

  nframes = fsize/blocksize;
  last_datablock_size = fsize - nframes*blocksize;
  ++nframes;

  *nframes_ptr = nframes;
  *last_datablock_size_ptr = last_datablock_size;

  /*
  fprintf(stdout, "fsize=%d, nframes=%d, last_datablock_size=%d\n",
	  fsize, nframes, last_datablock_size);
  */
  
  return(0);
}

int load_file(char *fname, int fsize, char *datap){

  int fd;
  int n;
  int status = 0;

  fd = open(fname, O_RDONLY);
  if(fd == -1)
    return(-1);
  
  n = read(fd, datap, fsize);
  close(fd);
  
  if(n != fsize)
    return(1);

  return(0);
}

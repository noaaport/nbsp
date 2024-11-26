/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 */
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
/* #include <stdio.h> */
#include "sbnpack.h"
#include "sbn.h"
#include "file.h"

static int get_file_stat_size(char *fname, int *fsize);

static int get_file_stat_size(char *fname, int *fsize){
  /*
   * Gets the size of the file, as stored on disk.
   */
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

int get_file_size(char *fname, int *fsize){
  /*
   * Gets the file size, as we will load it, including the ccb.
   */
  int stat_fsize;
  int status = 0;

  status = get_file_stat_size(fname, &stat_fsize);
  if(status != 0)
    return(status);

  *fsize = stat_fsize + CCB_SIZE;

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
  /*
   * The file stored on disk does not have the ccb. Here we add it
   * before loading the file. 
   */
  int fd;
  int n;
  
  fd = open(fname, O_RDONLY);
  if(fd == -1)
    return(-1);

  memset(datap, '\0', CCB_SIZE);
  datap[0] = CCB0;
  datap[1] = CCB1;
  
  n = read(fd, &datap[CCB_SIZE], fsize - CCB_SIZE);
  close(fd);
  
  if(n + CCB_SIZE != fsize)
    return(1);

  return(0);
}

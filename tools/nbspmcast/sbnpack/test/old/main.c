#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include "file.h"
#include "sbnpack.h"

static int save_frame(char *fname, struct sbnpack_st *sbnpack, int findex);

static int save_frame(char *fname, struct sbnpack_st *sbnpack, int findex){

  int fd;
  int n;
  int frame_size;
  int status = 0;

  frame_size = sbnpack->sbnpack_frame[findex].header_size +
    sbnpack->sbnpack_frame[findex].datablock_size;
  
  unlink(fname);
  fd = open(fname, O_WRONLY | O_CREAT, 0666);
  if(fd == -1)
    return(-1);

  n = write(fd, sbnpack->sbnpack_frame[findex].frame, frame_size);
  if(n == -1)
    status = -1;
  else if(n != frame_size){
    fprintf(stdout, "X: %d, %d\n", n, frame_size);
    status = 1;
  }

  close(fd);
  
  return(status);
}  

int main(int argc, char **argv){

  struct sbnpack_st gsbnpack;
  char *fname;
  int i;
  int nframes;
  int framesize;
  int fdout;
  int status = 0;
  
  if(argc < 2)
    errx(1, "%s", "Filename?");
  
  fname = argv[1];

  status = init_sbnpack(fname, &gsbnpack);
  if(status == -1)
    err(1, "%s", "OS Error");
  else if(status == 1)
    errx(1, "%s", "File too large");

  nframes = gsbnpack.nframes;

  fprintf(stdout, "fsize=%d, nblocks=%d, last_datablock_size=%d\n",
	  gsbnpack.sbnpack_file.data_size,
	  gsbnpack.nframes,
	  gsbnpack.sbnpack_frame[nframes - 1].datablock_size);

  unlink("output");
  fdout = open("output", O_WRONLY | O_CREAT, 0666);
  if(fdout == -1)
    err(1, "%s", "Error opening output file");

  for(i = 0; i < nframes; ++i){
    fprintf(stdout, "frame=%d, blocksize=%d\n",
	    i, gsbnpack.sbnpack_frame[i].datablock_size);
    
    status = write(fdout,
		   gsbnpack.sbnpack_frame[i].datablock,
		   gsbnpack.sbnpack_frame[i].datablock_size);
  }
  close(fdout);
    
  /*
   * Print the first, second, and last frames, in separate files
   */
  status = save_frame("output0", &gsbnpack, 0);
  if(status == -1)
    err(1, "%s", "Error from save_frame");
  else if(status != 0)
    errx(1, "%s", "Error from save_frame");
    
  if(nframes > 1) {
      status = save_frame("output1", &gsbnpack, 1);
      if(status == -1)
	err(1, "%s", "Error from save_frame");
      else if(status != 0)
	errx(1, "%s", "Error from save_frame");
  }

  if(nframes > 2) {
      status = save_frame("outputlast", &gsbnpack, nframes - 1);
      if(status == -1)
	err(1, "%s", "Error from save_frame");
      else if(status != 0)
	errx(1, "%s", "Error from save_frame");
  }
        
  return(0);
}

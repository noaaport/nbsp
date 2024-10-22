Essentially:

  char *fname;
  int nframes,
  int framesize;
  struct sbnpack_st gsbnpack;
  
  status = init_sbnpack(fname, &gsbnpack);
  nframes = gsbnpack.nframes;
  for(i = 0; i < nframes; ++i){
    framesize = gsbnpack.sbnpack_frame[i].datablock_size +
		gsbnpack.sbnpack_frame[i].header_size);

    status = write(fd, gsbnpack.sbnpack_frame[i].frame, framesize);
  }

This is implemented in the the function

  int send_sbnpack(int fd, struct sbnpack_st *sbnpack);

in util.c

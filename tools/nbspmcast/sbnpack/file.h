#ifndef FILE_H
#define FILE_H

int get_file_size(char *fname, int *fsize);
int get_file_frame_params(int blocksize, int fsize,
			  int *nframes, int *last_datablock_size);
int load_file(char *fname, int fsize, char *datap);

#endif

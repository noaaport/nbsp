/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef MFILE_H
#define MFILE_H

struct memfile_st {
  char *p;
  int maxsize;
  int size;	/* the writing pointer is just (p + size) */
  int nread;	/* how much has been read; reading pointer is (p + nread) */
};

struct memfile_st *open_memfile(int numblocks, int blocksize);
void close_memfile(struct memfile_st *mf);
int write_memfile(struct memfile_st *mf, void *data, int data_size);
int read_memfile(struct memfile_st *mf, void **pdata, int data_size);
int save_memfile(int fd, struct memfile_st *mf);
/* frame sequences */
int write_memframe(struct memfile_st *mf, 
		  char *frdata, int frdata_size, int f_compress);

int write_memframe_info(struct memfile_st *mf, 
		       int total_size, int f_compress);

int write_memframe_data(struct memfile_st *mf, char *data, int data_size); 
int get_memframe(struct memfile_st *mf, char **frdata, int *f_compress);

#endif

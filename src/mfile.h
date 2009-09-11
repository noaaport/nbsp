/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef MFILE_H
#define MFILE_H

/*
 * The maxallocsize parameter is used only for deallocating some memory
 * when it is possible. See adjust_memfile_size().
 */
struct memfile_st {
  size_t allocsize;	/* current allocation for p */
  size_t maxallocsize;	/* maximum allocation it has had in its lifetime */
  size_t size;		/* the writing pointer is just (p + size) */
  size_t nread;    /* how much has been read; reading pointer is (p + nread) */
  char *p;
};

struct memfile_st *create_memfile(size_t memfile_size);
void destroy_memfile(struct memfile_st *mf);
int realloc_memfile(struct memfile_st *mf, size_t newsize);
int open_memfile(struct memfile_st *mf);
void close_memfile(struct memfile_st *mf);
int write_memfile(struct memfile_st *mf, void *data, int data_size);
int write_memfile_fixed(struct memfile_st *mf, void *data, int data_size);
int read_memfile(struct memfile_st *mf, void **pdata, int data_size);
int save_memfile(int fd, struct memfile_st *mf);
int save_memfile_skip(int fd, struct memfile_st *mf, size_t skip);
size_t get_memfile_allocated_size(struct memfile_st *mf);
size_t get_memfile_data_size(struct memfile_st *mf);
int adjust_memfile_size(struct memfile_st *mf, size_t estimated_size);

/* frame sequences */
int write_memframe(struct memfile_st *mf, 
		  char *frdata, int frdata_size, int f_compress);

int write_memframe_info(struct memfile_st *mf, 
		       int total_size, int f_compress);

int write_memframe_data(struct memfile_st *mf, char *data, int data_size); 
int get_memframe(struct memfile_st *mf, char **frdata, int *f_compress);

#endif

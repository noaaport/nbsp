/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SBNPACK_H
#define SBNPACK_H

/*
 * We will divide the product data in blocks of size 4000. The first
 * frame will have the (flh + pdh + psh + ccb) which adds
 * (16 + 16 + 36 + 24) bytes for a total of (4000 + 92 = 4092).
 * The other frames have only the (flh + pdh).
 */
#define DATA_BLOCK_SIZE 4000
#define FRAME_LEVEL_HEADER_SIZE 16
#define PRODUCT_DEF_HEADER_SIZE 16
#define PRODUCT_SPEC_HEADER_SIZE 36
#define CCB_SIZE 24
#define HEADER_SIZE   32	/* flh + pdh */
#define HEADER_SIZE_0 92	/* flh + pdh + psh + ccb */
#define FRAME_SIZE_MAX 4092	/* data + header of first frame */

/*
 * The actual nymber of bytes in each frame is the sum of
 * the header_size and the datablock_size
 * (framesize = header_size + datablock_size).
 */
struct sbnpack_frame_st {
  char frame[FRAME_SIZE_MAX]; /* sbn frame transmitted by multicast */
  char *flh;	/* pointer to flh in frame[] */
  char *pdh;	/* pointer to pdh in frame[] */
  char *psh;	/* pointer to psh in frame[] */
  char *ccb;	/* pointer to ccb in frame[] */
  char *datablock; /* pointer to product data in this frame[] */
  int datablock_size;	/* size of the product data in this frame */
  int header_size;	/* size of header in this frame */
  int nframes;
  int frame_index; /* index of this frame */
};

struct sbnpack_file_st {
  char *data;
  int allocated_size;	/* allocated size */
  int data_size;	/* actual size of data stored (file size) */
  char *readp;		/* reading pointer */
};

struct sbnpack_st {
  struct sbnpack_file_st sbnpack_file;
  struct sbnpack_frame_st *sbnpack_frame;	/* array of sbn frames */
  int nframes;					/* # elements of the array */
};

int init_sbnpack_file(char *fname, struct sbnpack_file_st *sbnpack_file);
int reinit_sbnpack_file(char *fname, struct sbnpack_file_st *sbnpack_file);
void end_sbnpack_file(struct sbnpack_file_st *sbnpack_file);

struct sbnpack_frame_st *create_sbnpack_frame_array(int nframes,
						    int last_frame_size);
void destroy_sbnpack_frame_array(struct sbnpack_frame_st *sbnpack_frame);

/*
 * We have yet to write these two  - They have to call the fill_xxx
 * functions in fill.c (also to be written).
 */
int init_sbnpack(char *fname, struct sbnpack_st *sbnpack);
void end_sbnpack(struct sbnpack_st *sbnpack);

/* in fill.c */
void fill_blockdata(struct sbnpack_st *sbnpack);
void fill_headers(struct sbnpack_st *sbnpack);

/* Utility functions */
int send_sbnpack_frame(int fd, struct sbnpack_st *sbnpack, int findex);
int send_sbnpack(int fd, struct sbnpack_st *sbnpack);

#endif

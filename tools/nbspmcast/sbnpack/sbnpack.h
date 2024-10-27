/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SBNPACK_H
#define SBNPACK_H

#include <stdint.h>

/*
 * We will divide the product data in blocks of size 4000.
 * We will include (add) the ccb[24] in the first data block.
 * (This is done by the functions get_file_frame_params()
 * and load_file() in file.c).
 * The first frame will have the (flh + pdh + psh) which adds
 * (16 + 16 + 36) bytes for a total of (4000 + 68 = 4068).
 * The other frames have only the (flh + pdh).
 */
#define DATA_BLOCK_SIZE 4000
#define FRAME_LEVEL_HEADER_SIZE 16
#define PRODUCT_DEF_HEADER_SIZE 16
#define PRODUCT_SPEC_HEADER_SIZE 36
#define HEADER_SIZE   32	/* flh + pdh */
#define HEADER_SIZE_0 68	/* flh + pdh + psh */
#define FRAME_SIZE_MAX 4068	/* data + header of first frame */

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
  char *datablock; /* pointer to data block in this frame[] */
  int datablock_size;	/* size of the data block in this frame */
  int header_size;	/* size of header in this frame (header length) */
  int nframes;
  int frame_index; /* index of this frame */
};

/*
 * The data_size is the file size (including the ccb)
 */
struct sbnpack_file_st {
  char *data;		/* includes the ccb */
  int allocated_size;	/* allocated size */
  int data_size;	/* actual size of data stored (file size) */
  char *readp;		/* reading pointer */
};

struct sbnpack_st {
  struct sbnpack_file_st sbnpack_file;
  struct sbnpack_frame_st *sbnpack_frame;	/* array of sbn frames */
  int nframes;					/* # elements of the array */
  uint32_t prod_seq_number;
  uint32_t sbn_seq_number;
  int psh_type_flag;
};

int init_sbnpack(struct sbnpack_st *sbnpack,
		 char *fname,
		 uint32_t prod_seq_number,
		 uint32_t sbn_seq_number,
		 int psh_type_flag);
void end_sbnpack(struct sbnpack_st *sbnpack);
int create_sbnpack(char *fname,
		   uint32_t prod_seq_number,
		   uint32_t sbn_seq_number,
		   int psh_type_flag,
		   struct sbnpack_st **sbnpack);
void free_sbnpack(struct sbnpack_st *sbnpack);


/* Utility functions */
int write_sbnpack_frame(int fd, struct sbnpack_st *sbnpack, int findex);
int write_sbnpack(int fd, struct sbnpack_st *sbnpack);

#endif

/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef NBS1_H
#define NBS1_H

#include <sys/types.h>
#include "const.h"
#include "pack.h"

/*
 * The format of the data portion is:
 *
 * byte = p;
 *
 * byte[0-3] - uint32 - seq_number
 * byte[4] - uchar - psh_product_type
 * byte[5] - uchar - psh_product_category
 * byte[6] - uchar - psh_product_code
 * byte[7] - uchar - np_channel_index
 * p += 8;
 * byte[0 - FBASENAME_SIZE] - fbasename (including '\0')
 * p += FBASENAME_SIZE + 1;
 * byte[0] - uchar - compressed flag
 * byte[1-2] - uint16 - num_blocks (n >= 1)
 * byte[3-4] - uint16 - block_number  (1 to n; not 0 to n - 1)
 * byte[5-8] - uint32 - file block size (how much more to read)
 * byte[9- ] -        - start of data
 *
 * - The uint32 values are stored with the the most significant digit in
 *   the first byte, the least significant digit in the last byte.
 *
 * - It is guaranteed that, before and after decompression, the data is never
 *   larger than the constant NBS1_BLOCK_SIZE.
 *
 * All this is preceeded by the envelope (pack.h).
 */
#define NBS1_HEADER_SIZE	(17 + FBASENAME_SIZE + 1)
#define NBS1_BLOCK_SIZE		65536	/* 64 K */
#define NBS1_PACKET_MAXSIZE	(NBS1_BLOCK_SIZE + NBS1_HEADER_SIZE \
				 + NBS_ENVELOPE_SIZE)

struct nbs1_packet_st {
  char packet[NBS1_PACKET_MAXSIZE];
  size_t packet_size;	/* envelope + header + file content */
  size_t block_size;	/* without header and envelope */
  int num_blocks;
  int block_number;	/* 1 to num_blocks */
  int last_block_size;  /* how much will be the last read from fd */
  int f_zip;
  int fsfd;		/* reading from the file system */
  int cfd;		/* reading from the spool cache */
  int mfd;		/* reading from the mspooldb */
  uint32_t seq_number;
  int psh_product_type;
  int psh_product_category;
  int psh_product_code;
  int np_channel_index;
  char fbasename[FBASENAME_SIZE + 1];
  /*
   * These variables are not used for transmission - they are used
   * by the receiving (slave) functions.
   * nbs->block points to the block data, past the envelope and header.
   * slavenbs_reader_index is set to the slave threads reader index into
   * the pctl.
   */
  char *block;
  int slavenbs_reader_index;
};

int init_nbs_packet_st(struct nbs1_packet_st *nbs, 
		       char *fpath, uint32_t seqnumber, 
		       int psh_product_type, int psh_product_category,
		       int psh_product_code, int np_channel_index,
		       char *fname);

void free_nbs_packet_st(struct nbs1_packet_st *nbs);
int build_nbs_packet(struct nbs1_packet_st *nbs);
int send_nbs_packet(int fd, struct nbs1_packet_st *nbs,
		    int timeout_ms, int retry);
int nbs_file_filter(char *fname);

/*
 * These functions are not used by the server but they are used by
 * nbsp in slave mode, abd can be used by any client.
 */
int unpack_nbs1_header(struct nbs1_packet_st *nbs);
int recv_nbs_packet(int fd, struct nbs1_packet_st *nbs,
		    unsigned int timeout_s, int retry);

#endif

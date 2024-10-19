/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SBN_H
#define SBN_H

#include "common.h"

/*
 * The maximum size of the data portion of an sbn frame is 5120. The
 * sbn frame includes at least the flh and pdh, and can include the psh,
 * which add 16 + 16 + 36.
 */
#define SBN_FRAME_SIZE		5200

/* 
 * This is the maximum size of the uncompressed block data. From ``RADAR
 * product compression approach'' (page 2): Also each decompressed SBN
 * data frame will not exceed the existing maximum SBN frame size
 * since that is the maximum frame size of the original data frame.
 * We could use 5120 here.
 */
/* #define MAX_FRDATA_SIZE		SBN_FRAME_SIZE (common.h) */

/* 
 * The ctrl header (of the first frame of the compressed data frames)
 * has a CCB and a WMO, 54 or 58 bytes total. The WMO starts at byte 24.
 * Apart from 24 bytes of the ccb,
 *
 * TTAAii CCCC YYGGgg[ BBB]\r\r\n = 21 [+4]
 * NNNxxx\r\r\n = 9
 *
 * In some (compressed) products (e.g., satellite images) the ctrlhdr
 * does not have the ccb part, only the wmo part, and without the awips line.
 */
/* #define CCB_SIZE             24   (common.h) */
/* #define CTRLHDR_WMO_SIZE	21   excluding possible [ BBB] and awips */
#define CTRLHDR_WMOAWIPS_SIZE	30   /* excluding possible [ BBB] */
#define MAX_CTRLHDR_SIZE        (CCB_SIZE + CTRLHDR_WMOAWIPS_SIZE + 4) /* 58 */

/* struct product_spec_header->product_types */
#define PSH_TYPE_GOESE		1
#define PSH_TYPE_GOESW		2
#define PSH_TYPE_NOAAPORT	3
#define PSH_TYPE_NWSTG		4
#define PSH_TYPE_NEXRAD		5

/* struct product_spec_header->transfer_status_flag */
#define PSH_STATUSFLAG_RETRANSMIT 16

/* struct product_def_header->transfer_type */
#define PDH_TRANSFERTYPE_START		1
#define PDH_TRANSFERTYPE_PROGRESS	2
#define PDH_TRANSFERTYPE_END		4
#define PDH_TRANSFERTYPE_ERROR		8
#define PDH_TRANSFERTYPE_COMPRESSED	16	/* guessed */
#define PDH_TRANSFERTYPE_ABORT		32
#define PDH_TRANSFERTYPE_OPTIONS	64

/* frame_level_header->sbn_command */
#define SBN_COMMAND_DATA_TRANSFER		3
#define SBN_COMMAND_SYNC_TIMING			5
#define SBN_COMMAND_TEST_MSG			10

/* frame_level_header->sbn_datstream */
#define SBN_DATSTREAM_GOESE			1
#define SBN_DATSTREAM_GOESW			2
#define SBN_DATSTREAM_NGOES			4
#define SBN_DATSTREAM_NWSTG			5

#define FRAME_LEVEL_HEADER_SIZE 16
#define PRODUCT_DEF_HEADER_SIZE 16
#define PRODUCT_SPEC_HEADER_SIZE 36

struct frame_level_header {
  int data_cksum;	/* our calculated checksum */
  int hdlc_address;
  int hdlc_control;
  int header_length;
  int sbn_version;
  int sbn_control;
  int sbn_command;
  int sbn_datstream;		/* identifies the channel(data stream) */
  int sbn_src;
  int sbn_dest;
  unsigned int sbn_seq_number;
  int sbn_run;
  int sbn_cksum;	/* ithe data  is only two bytes */
};

struct product_def_header{
  int version;
  int transfer_type;
  int header_length32;		/* length in 32 bit words */
  int header_length;		/* length in bytes */
  int block_number;
  int data_block_offset;
  int data_block_size;
  int records_per_block;
  int blocks_per_record;
  unsigned int product_seq_number;
};

struct product_spec_header{
  int opt_field_number;
  int opt_field_type;
  int opt_field_length;
  int version;
  int transfer_status_flag;
  int awips_data_length;
  int bytes_per_record;	/* For GOES imgs, is the num of bytes per scan line */
  int product_type;
  int product_category;
  int product_code;
  int num_fragments;
  int next_header_offset;	/* reserved for future consideration */
  int reserved;
  int source;
  unsigned int orig_seq_number;
  unsigned int ncf_recv_time;
  unsigned int ncf_transmit_time;
  int run_id;
  int orig_run_id;		/* used during retransmit */
};

struct sbn_frame {
  char rawdata[SBN_FRAME_SIZE];
  int rawdata_size;		  /* size actually read */
  struct frame_level_header fh;
  struct product_def_header pdh;
  struct product_spec_header psh;
  int np_channel_index;	 /* (our) noaaport channel (0-3) [set in reader.c] */
  char *ctrlhdr;    /* ptr to product-data header (copy of ccb+wmo) */
  int ctrlhdr_size;
  char *frdata;		     /* ptr to product data (possibly compressed ) */
  int frdata_size;
  char blkdata[MAX_FRDATA_SIZE]; /* uncompressed product data (has ccb+wmo) */
  int blkdata_size;
  char *ccb;		      /* ptr to ccb header of blkdata */
  int ccb_size;		      /* set to 0 if there is no cbb */
  char *pdata;		      /* ptr to start of wmo data file (after ccb) */
  int pdata_size;	      /* blkdata_size - cbb_size */
  int f_frdata_compressed;	/* we set it to 1 if frdata is compressed */
  int f_unzstatus;		/* status returned by zlib uncompress */
};  

/* this function takes care of all the sbn protocol */
int sbn_unpack_frame(struct sbn_frame *frame);

/* utility functions for the nbsp */
int split_wmo_header(struct sbn_frame *sbnf,
		     char *wmo_id, char *wmo_station, 
		     char *wmo_time, char *wmo_awips, char *wmo_notawips);
int copy_ctrlheader(char *ctrlheader, int *ctrlheader_size,
		      struct sbn_frame *sbnf);

#endif

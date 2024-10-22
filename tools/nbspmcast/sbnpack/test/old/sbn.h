/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 */
#ifndef SBN_H
#define SBN_H

#include <stdint.h>

/*
 * The "defines" here are copied from those used in the nbsp
 * src/sbn.h file. The definitions of the various header
 * structures are copied from the same file, but here
 * we have used the correct size of the structute elements, to
 * document explicitly the packaging of the values in the frame[].
 */

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

/* frame_level_header->sbn_version */
/* The most significant 4 bits gives the sbn version. The least significant
 * 4 bits give the flh length (in 32 bit words). I will use 1 for the version
 * and 4 for the flh length (16 bytes): (1 << 4) + 4.
 */
#define SBN_VERSION 20

/* product_def_header->version */
/* Similar to the above, but with the "product definition version */
#define PDH_VERSION 20

void fill_flh(struct sbnpack_frame_st *sbnpack_frame,
	      uint32_t sbn_seq_number);
void fill_pdh(struct sbnpack_frame_st *sbnpack_frame,
	      uint32_t prod_seq_number);
	      
#endif

/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 */
#include "util.h"
#include "sbnpack.h"
#include "sbn.h"
#include <assert.h>

void fill_flh(struct sbnpack_frame_st *sbnpack_frame,
		     uint32_t sbn_seq_number){
  /*
   * sbn_seq_number is the sequence number for each frame
   * (nbsp does not use this, but we will set it here anyway for completeness).
   */
  uint16_t cksum;
  unsigned char *p = (unsigned char*)(sbnpack_frame->flh);
  int i;

  p[0] = 0xff;	  /* HDLC address - all 1 */
  p[1] = 0;	  /* HDLC record - not used */
  p[2] = SBN_VERSION; /* see sbn.h */
  p[3] = 0;		/* SBN control */

  p[4] = SBN_COMMAND_DATA_TRANSFER; /* SBN command */
  p[5] = SBN_DATSTREAM_NWSTG;		/* data stream */
  p[6] = 1;		/* source (NCF) */
  p[7] = 0;		/* destination (All = 0) */

  /* the seq number is packed in bytes 8-11 */
  /* p[8] = (sbn_seq_number >> 24) & 0xff;
     p[9] = (sbn_seq_number >> 16) & 0xff;
     p[10] = (sbn_seq_number >> 8) & 0xff;
     p[11] = sbn_seq_number & 0xff;
  */
  pack_uint32(p, sbn_seq_number, 8);

  /* run identifier */
  /*  p[12] = (1 >> 8) & 0xff;
      p[13] = 1 & 0xff;
  */
  pack_uint16(p, 1, 12);
    
  /*
   * The checksum
   */
  cksum = 0;
  for(i = 0; i < FRAME_LEVEL_HEADER_SIZE - 2; ++i)
    cksum += p[i];

  /*
    p[14] = (cksum >> 8) & 0xff;
    p[15] = cksum & 0xff;
  */
  pack_uint16(p, cksum, 14);
  
}

void fill_pdh(struct sbnpack_frame_st *sbnpack_frame,
	      uint32_t prod_seq_number){
  /*
   * prod_seq_number is the sequence number for the product.
   */
  int nframes = sbnpack_frame->nframes;
  uint16_t frame_index = sbnpack_frame->frame_index;
  uint16_t header_length = sbnpack_frame->header_size;
  uint16_t datablock_size = sbnpack_frame->datablock_size;
  unsigned char *p = (unsigned char*)(sbnpack_frame->pdh);

  p[0] = PDH_VERSION;		/* see sbn.h */

  /* p[1] = Transfer type: start=1, progress=2, end=4 */
  p[1] = 0;
 
  if(frame_index == 0)
    p[1] += PDH_TRANSFERTYPE_START;

  p[1] += PDH_TRANSFERTYPE_PROGRESS;

  if(frame_index == nframes - 1)
    p[1] += PDH_TRANSFERTYPE_END;

  /* p[2,3] - header length = sbnpack_frame->header_size - set above */
  /*
   p[2] = (header_length >> 8) & 0xff;
   p[3] = header_length & 0xff;
  */
  pack_uint16(p, header_length, 2);

  /*
   * bytes 6,7 have the "data block offset". The doc is vague. What
   * it really means is whether there is a ctrlheader (that precedes
   * the data file) or not. txt files do not have a ctrlheader so the
   * offset is 0.
   */
  pack_uint16(p, frame_index, 4);     /* p[4,5] - block number (from 0 to n) */
  pack_uint16(p, 0, 6);		      /* p[6,7] - no ctrlheader => offset = 0 */
  pack_uint16(p, datablock_size, 8);  /* p[8,9] - data block size */
  p[10] = 1;			      /* #records within data block */
  p[11] = 1;			      /* #blocks that a record spans */
  
  pack_uint32(p, prod_seq_number, 12);  /* p[12-15] - seq num */
}

void fill_psh(struct sbnpack_frame_st *sbnpack_frame,
	      int psh_type_flag) {

  int nframes = sbnpack_frame->nframes;
  uint16_t frame_index = sbnpack_frame->frame_index;
  unsigned char *p = (unsigned char*)(sbnpack_frame->psh);

  /* Only the first frame has a psh */
  assert(frame_index == 0);

  /* option field: number, type, length */
  p[0] = 0;
  p[1] = 0;
  pack_uint16(p, 0, 2);    /* p[2,3] - option field length */

  p[4] = 1;		   /* psh version */
  
  /* p[5] is a flag - similar to p[1] in pdh */
  p[5] = 0;
  if(frame_index == 0)
    p[5] += PSH_STATUSFLAG_START;

  p[5] += PSH_STATUSFLAG_PROGRESS;

  if(frame_index == nframes - 1)
    p[5] += PSH_STATUSFLAG_END;

  pack_uint16(p, PRODUCT_SPEC_HEADER_SIZE, 6);    /* p[6,7] - length of psh */
  pack_uint16(p, 0, 8);		/* p[8,9] - #bytes per scan line for GOES */

  /* psh type */
  p[10] = PSH_TYPE_NWSTG;	/* default */
  if(psh_type_flag != 0)
    p[10] = PSH_TYPE_NEXRAD;

  p[11] = 0;			/* ps category */
  pack_uint16(p, 0, 12);	/* p[12,13] - product code - from 0-255 */

  pack_uint16(p, nframes, 14);	/* p[14,15] - # of "fragments" */
  pack_uint16(p, 0 , 16);	/* p[16,17] next header offset - reserved */
  p[18] = 0;			/* reserved */
  p[19] = 0;			/* product source - NWSTG, ... */
  pack_uint32(p, 0, 20);	/* 0 or orig product seqnum in a retransmit */
  pack_uint32(p, 0, 24);	/* time received at ncf */
  pack_uint32(p, 0, 28);	/* time transmitted from ncf */
  pack_uint16(p, 0, 32);	/* run identifier */
  pack_uint16(p, 0, 34);	/* orig run id - used in retransmit */
}

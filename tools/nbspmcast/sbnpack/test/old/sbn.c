#include "util.h"
#include "sbnpack.h"
#include "sbn.h"

void fill_flh(struct sbnpack_frame_st *sbnpack_frame,
		     uint32_t sbn_seq_number){
  /*
   * sbn_seq_number is the sequence number for each frame
   * (nbsp does not use this, but we will set it here anyway for completeness).
   */
  uint16_t cksum;
  unsigned char *p = (unsigned char*)(sbnpack_frame->flh);
  int i;

  p[0] = 0xf;	  /* HDLC address */
  p[1] = 0;	  /* HDLC record */
  p[2] = SBN_VERSION; /* see sbn.h */
  p[3] = 0;		/* SBN control */

  p[4] = SBN_COMMAND_DATA_TRANSFER; /* SBN command */
  p[5] = SBN_DATSTREAM_NWSTG;		/* data stream (GOES, ...) */
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
  int frame_index = sbnpack_frame->frame_index;
  unsigned char *p = (unsigned char*)(sbnpack_frame->fdh);

  pdfh->transfer_type = data[1];
  pdfh->header_length = (data[2] << 8) + data[3];		
  pdfh->block_number = (data[4] << 8) + data[5];
  pdfh->data_block_offset = (data[6] << 8) + data[7];
  pdfh->data_block_size = (data[8] << 8) + data[9];
  pdfh->records_per_block = data[10];
  pdfh->blocks_per_record = data[11];
  pdfh->product_seq_number = (data[12] << 24) + (data[13] << 16) +
    (data[14] << 8) + data[15];

  p[0] = PDF_VERSION;		/* see sbn.h */

  /* p[1] = Transfer type: start=1, progress=2, end=4 */
  if(frame_index == 0)
    p[1] = PDH_TRANSFERTYPE_START;

  if(frame_index > 0)
    p[1] += PDH_TRANSFERTYPE_PROGRESS;

  if(frame_index == nframes -1)
    p[1] += PDH_TRANSFERTYPE_END;

  if(findex == 0)
  p[2] = FRAME_LEVEL_HEADER_SIZE
#define PRODUCT_DEF_HEADER_SIZE 16
#define PRODUCT_SPEC_HEADER_SIZE 36
;	/* "total" length of product header for this frame */
    
}

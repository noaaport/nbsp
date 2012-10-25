/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "unz.h"
#include "sbn.h"
#include "const.h"

static int unpack_sbn_headers(struct sbn_frame *sbnf);
static int unpack_fl_header(struct frame_level_header *flh, 
			    unsigned char *data);
static void unpack_pd_header(struct product_def_header *pdh, 
			     unsigned char *data);
static void unpack_ps_header(struct product_spec_header *psh, 
			      unsigned char *data);

static int unpack_fl_header(struct frame_level_header *flh, 
			    unsigned char *data){
  /*
   * Returns:
   *          0 if our calculated checksum equals the transmited checksum
   *          1 otherwise
   */
  int version;
  int i;

  flh->hdlc_address = data[0];
  flh->hdlc_control = data[1];

  version = data[2];
  flh->header_length = version & 0xf;
  flh->sbn_version = (version >> 4) & 0xf;

  flh->sbn_control = data[3];
  flh->sbn_command = data[4];
  flh->sbn_datstream = data[5];
  flh->sbn_src = data[6];
  flh->sbn_dest = data[7];

  flh->sbn_seq_number = (data[8] << 24) + (data[9] << 16) +
    (data[10] << 8) + data[11];

  flh->sbn_run = (data[12] << 8) + data[13];

  flh->sbn_cksum = (data[14] << 8) + data[15];

  /* Our calculated checksum - includes all bytes except the last two */
  flh->data_cksum = 0;
  for(i = 0; i < FRAME_LEVEL_HEADER_SIZE - 2; ++i)
    flh->data_cksum += data[i];

  return(flh->sbn_cksum == flh->data_cksum? 0: 1);
}

static void unpack_pd_header(struct product_def_header *pdfh, 
			     unsigned char *data){
  int version;

  version = data[0];
  pdfh->header_length32 = version & 0xf;
  pdfh->version = (version >> 4) & 0xf;
  
  pdfh->transfer_type = data[1];
  pdfh->header_length = (data[2] << 8) + data[3];		
  pdfh->block_number = (data[4] << 8) + data[5];
  pdfh->data_block_offset = (data[6] << 8) + data[7];
  pdfh->data_block_size = (data[8] << 8) + data[9];
  pdfh->records_per_block = data[10];
  pdfh->blocks_per_record = data[11];
  pdfh->product_seq_number = (data[12] << 24) + (data[13] << 16) +
    (data[14] << 8) + data[15];
}

static void unpack_ps_header(struct product_spec_header *psh, 
			     unsigned char *p){

  psh->opt_field_number = p[0];
  psh->opt_field_type = p[1];
  psh->opt_field_length = (p[2] << 8) + p[3];
  psh->version = p[4];
  psh->transfer_status_flag = p[5];
  psh->awips_data_length = (p[6] << 8) + p[7];
  psh->bytes_per_record = (p[8] << 8) + p[9];
  psh->product_type = p[10];
  psh->product_category = p[11];
  psh->product_code = (p[12] << 8) + p[13];
  psh->num_fragments = (p[14] << 8) + p[15];
  psh->next_header_offset = (p[16] << 8) + p[17];
  psh->reserved = p[18];
  psh->source = p[19];
  psh->orig_seq_number = (p[20] << 24) + (p[21] << 16) + (p[22] << 8) + p[23];
  psh->ncf_recv_time = (p[24] << 24) + (p[25] << 16) + (p[26] << 8) + p[27];
  psh->ncf_transmit_time = (p[28] << 24) + (p[29] << 16) + (p[30] << 8) + 
    p[31];
  psh->run_id = (p[32] << 8) + p[33];
  psh->orig_run_id = (p[34] << 8) + p[35];
}

static int unpack_sbn_headers(struct sbn_frame *sbnf){
  /*
   * Returns 
   *	1 if there is checksum error.
   *    2 if the frame is inconsistent
   *	0 otherwise
   */
  int blkdata_offset;
  int cksum_status = 0;
  unsigned char *p;
  int status = 0;

  blkdata_offset = FRAME_LEVEL_HEADER_SIZE;
  if(sbnf->rawdata_size < blkdata_offset)
    return(2);

  p = (unsigned char*)&sbnf->rawdata[0];
  cksum_status = unpack_fl_header(&sbnf->fh, p);
  if(cksum_status != 0)
    return(1);

  if(sbnf->fh.sbn_command != SBN_COMMAND_DATA_TRANSFER){
    /*
     * When the sbn command is 5 (time sychronization), the size
     * of the raw data is 48. The sbn header (just decoded) is 16.
     * I don't know what is the rest, but it does not appear to be
     * a pdh or psh.
     */
    return(0);
  }

  blkdata_offset += PRODUCT_DEF_HEADER_SIZE;
  if(sbnf->rawdata_size < blkdata_offset)
    return(2);

  p = (unsigned char*)&sbnf->rawdata[FRAME_LEVEL_HEADER_SIZE];
  unpack_pd_header(&sbnf->pdh, p);

  /*
   * Only the first frame has a PS header
   */
  if(sbnf->pdh.block_number == 0){
    blkdata_offset += PRODUCT_SPEC_HEADER_SIZE;

    if(sbnf->rawdata_size < blkdata_offset)
      return(2);

    p = (unsigned char*)&sbnf->rawdata[FRAME_LEVEL_HEADER_SIZE
				       + PRODUCT_DEF_HEADER_SIZE];
    unpack_ps_header(&sbnf->psh, p);
  }

  if(sbnf->rawdata_size != blkdata_offset + sbnf->pdh.data_block_size)
    return(2);

  return(status);
}

int sbn_unpack_frame(struct sbn_frame *sbnf){
  /*
   * Returns 
   *    Same codes as unpack_sbn_headers().
   *	3 if uncompress gives an error (sbnf->f_unzstatus will have the code)
   */
  int blkdata_offset;
  char *pdata;
  int pdata_size;
  int status = 0;

  status = unpack_sbn_headers(sbnf);
  if((status != 0) || (sbnf->fh.sbn_command != SBN_COMMAND_DATA_TRANSFER))
    return(status);

  blkdata_offset = FRAME_LEVEL_HEADER_SIZE + PRODUCT_DEF_HEADER_SIZE;

  if(sbnf->pdh.block_number == 0)
    blkdata_offset += PRODUCT_SPEC_HEADER_SIZE;

  /*
   * The data portion of the frame may or may not have a "block header"; that
   * contains (a copy of) the CCB and the WMO header.
   */
  if(sbnf->pdh.data_block_offset == 0){
    sbnf->ctrlhdr = NULL;
    sbnf->ctrlhdr_size = 0;
  }else{
    sbnf->ctrlhdr = &sbnf->rawdata[blkdata_offset];
    sbnf->ctrlhdr_size = sbnf->pdh.data_block_offset;

    if(sbnf->ctrlhdr_size < CTRLHDR_WMO_SIZE){
      /*
       * log_info("Frame with a ctrlhdr smaller than a wmo header.");
       */
      return(2);
    }
  }

  blkdata_offset += sbnf->ctrlhdr_size;
  sbnf->frdata_size = sbnf->pdh.data_block_size - sbnf->ctrlhdr_size;
  sbnf->frdata = &sbnf->rawdata[blkdata_offset];

  if((sbnf->pdh.transfer_type & PDH_TRANSFERTYPE_COMPRESSED) != 0)
    sbnf->f_frdata_compressed = 1;
  else
    sbnf->f_frdata_compressed = 0;

  /*
   * After the control header, comes the noaaport product data. It may or 
   * may not be compressed. If it is compressed and there was no
   * sbnf->ctrlhdr (above), we uncompress it and store the
   * uncompressed copy in the blkdata[] array if it is the first fragment
   * of a product. If it is not the first fragment, we will queue
   * the frdata (whether it is compressed or not) so blkdata[] is not used.
   *
   * The condition that a compressed frame is transmitted without a ctrlhdr
   * should not actually happen, but we check it and proceed accordingly
   * as a safety measure; we could instead return an error 2.
   */

  if((sbnf->pdh.block_number == 0) && (sbnf->ctrlhdr == NULL)){
    if(sbnf->f_frdata_compressed == 1){
      /*
       * log_info("Compressed frame transmitted without ctrlhdr.");
       */
      sbnf->blkdata_size = MAX_FRDATA_SIZE; 
      sbnf->f_unzstatus = unz(sbnf->blkdata, &sbnf->blkdata_size, 
			      sbnf->frdata, sbnf->frdata_size);
      if(sbnf->f_unzstatus != 0)
	return(3);
    }
  }

  /*
   * The uncompressed blockdata in the _first_ frame is embedded in a 
   * ccb + wmo + awips header or a wmo + nesdis header, depending
   * on the product and/or channel. 
   * The ccb header starts with a '@'. If that is absent, there is no ccb
   * and the first line contains only the WMO. Instead of checking
   * this way, we can also try to do it according to the transmission
   * channel, but I am not following the first approach until I am sure
   * each channel has unique specification for this.
   */

  if(sbnf->pdh.block_number != 0){
    /*
     * The other frames are pure data.
     */
    sbnf->ccb = NULL;
    sbnf->ccb_size = 0;

    return(status);
  }

  /*
   * Only the first frame reaches this.
   */

  if(sbnf->ctrlhdr != NULL){
    if(isalpha(sbnf->ctrlhdr[0])){
      sbnf->ccb = NULL;
      sbnf->ccb_size = 0;
    }else{
      if(sbnf->ctrlhdr_size < CCB_SIZE)
	return(2);
      else{
	sbnf->ccb = &sbnf->ctrlhdr[0];
	sbnf->ccb_size = CCB_SIZE;
      }
    }
    sbnf->pdata = NULL;
    sbnf->pdata_size = 0;
  }else{
    /*
     * No ctrlhdr. Use the frdata when the frame is uncompressed, or
     * the decompressed blkdata when it was compressed.
     */
    if(sbnf->f_frdata_compressed == 1){
      pdata = sbnf->blkdata;
      pdata_size = sbnf->blkdata_size;
    }else{
      pdata = sbnf->frdata;
      pdata_size = sbnf->frdata_size;
    }

    if(isalpha(pdata[0])){
      sbnf->ccb = NULL;
      sbnf->ccb_size = 0;
    }else{
      sbnf->ccb = pdata;
      sbnf->ccb_size = CCB_SIZE;
      if(pdata_size < CCB_SIZE)
	return(2);
    }    
    sbnf->pdata = &pdata[sbnf->ccb_size];
    sbnf->pdata_size = pdata_size - sbnf->ccb_size;
  }

  return(status);
}

int copy_ctrlheader(char *ctrlheader, int *ctrlheader_size,
		      struct sbn_frame *sbnf){
  /*
   * The "ctrlheader" is a copy of the CCB, WMO and AWIPS headers in the file.
   * Is is sent (sometimes it is not sent) as a block preceedeing the
   * the product file. I have no use for it, but I am keeping 
   * in the pctl in case it is useful in the future.
   * When calling the function, ctrlheader_size should be initalized
   * allocated size of the buffer.
   *
   * Returns:
   *   1 => no control header
   *  -1 => memory error
   *   0 => no error
   */
  int allocatedsize;

  allocatedsize = *ctrlheader_size;
  *ctrlheader_size = 0;
  ctrlheader[0] = '\0';
 
  if(sbnf->ctrlhdr == NULL)
    return(1);

  if(sbnf->ctrlhdr_size > allocatedsize){
    /*
     * If the actual size is larger than the allocated size passed to
     * the function, we treat it is a non existent, although this is a bug
     * in the allocation of the size of the buffer.
     */
    assert(allocatedsize >= sbnf->ctrlhdr_size);

    return(1);
  }

  memcpy(ctrlheader, sbnf->ctrlhdr, sbnf->ctrlhdr_size);
  *ctrlheader_size = sbnf->ctrlhdr_size;

  return(0);
}

int split_wmo_header(struct sbn_frame *sbnf,
		      char *wmo_id, char *wmo_station, 
		      char *wmo_time, char *wmo_awips, char *wmo_notawips){
  /*
   * This function extracts the WMO/AWIPS header from the control header
   * if there is one in the transmission, or the WMO from the first line
   * of the file and the AWIPS code from the second line.
   * If the second line does not have the awips code, it extracts 
   * the first (up to 6) ascii characters it finds there 
   * (usually GRIB, BUFR, METAR, etc) if they are there.
   *
   * Returns:
   *	0 => ok
   *    1 => some error in the first line
   *
   *    NOTE: If there is no awips line, then upon return wmo_awips[0] 
   *    contains '\0'. Same with wmo_nawips[0].
   */
  char *wmo;
  int i;
  int b;
  int n;
  int data_size;

  if(sbnf->ctrlhdr != NULL){
    if(isalpha(sbnf->ctrlhdr[0])){
      wmo = &sbnf->ctrlhdr[0];
      data_size = sbnf->ctrlhdr_size;
    }else{
      wmo = &sbnf->ctrlhdr[CCB_SIZE];
      data_size = sbnf->ctrlhdr_size - CCB_SIZE;
    }

    if(data_size < CTRLHDR_WMO_SIZE){
      /*      
       * char buffer[64];
       * strncpy(buffer, wmo, data_size);
       * buffer[data_size] = '\0';
       * log_info("XXX %s", buffer);
       */
      return(1);
    }
  }else{
    /* pdata points after the ccb, if there is any */
    wmo = sbnf->pdata;
    data_size = sbnf->pdata_size;
  }

  if(sscanf(wmo, "%6s %4s %6s", wmo_id, wmo_station, wmo_time) != 3)
    return(1);

  /* In case we find nothing */
  wmo_awips[0] = '\0';
  wmo_notawips[0] = '\0';

  /*
   * start of the awips line, if any
   */
  i = 0;
  while((*wmo != '\n') && (i < data_size)){
     ++wmo;
     ++i;
  }

  if(i >= data_size - 1){
    /*
     * There is nothing.
     */
    return(0);
  }

  /*
   * Assuming we hit the \n, we have scanned (i + 1) characters,
   * including the \n. Thus he have to scan the rest, which are
   * data_size - (i + 1). First, point beyond the \n
   */

  ++wmo;

  /*
   * The line must contain at least the 6 characters of the awips
   * (some of them could be blanks) plus two \r plus a final \n
   * if it has an awips code, or some characters followed by a non
   * isalnum() character.
   */

  if(isalpha(wmo[0]) == 0){
    /*
     * There are no ascii characters.
     */
    return(0);
  }

  n = data_size - (i + 1);
  if((wmo[8] == '\n') && (n >= 9)){
    if(sscanf(wmo, "%6s", wmo_awips) != 1)
      wmo_awips[0] = '\0';
  }else{
    b = 0;
    while((b < WMO_NOTAWIPS_SIZE) && (b < n)){
      wmo_notawips[b] = wmo[b];

      if(isalnum(wmo_notawips[b]) == 0)
	break;
      
      ++b;
    }
    wmo_notawips[b] = '\0';
  }

  return(0);
}



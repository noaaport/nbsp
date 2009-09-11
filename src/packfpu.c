/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "util.h"
#include "packfpu.h"

/*
 * These are the functions used to unpack the data that is sent
 * to the server and filter queues (the fpath of the file, NBS2). 
 */

int nbsfp_unpack_fpath(struct packet_info_st *packetinfo){
  /*
   * This function does the opposite of the pack function.
   * It is assumed that packetinfo->packet and packetinfo->packet_size
   * are known, and then this function recovers the other elements
   * of the structure from that.
   */
  int status = 0;
  unsigned int dataid;
  int data_size;
  uint32_t transmitted_checksum;
  uint32_t calculated_checksum;
  char *p;

  p = (char*)packetinfo->packet;

  dataid = unpack_uint32(p, 0);
  /* assert(dataid == PACKID_FPATH); */
  if(dataid != PACKID_FPATH)
    return(1);

  data_size = unpack_uint32(p, 4);
  transmitted_checksum = unpack_uint32(p, 8);

  /*
   * The rest of the message (data_size) is the full path name of the file 
   * (including a trailing '\0'), preceeded by the product data codes
   * (which includes the fname).
   */
  p += NBS_ENVELOPE_SIZE;

  calculated_checksum = calc_checksum(p, data_size);
  if(calculated_checksum != transmitted_checksum){
    return(1);
  }

  /*
   *  p[0-7] contain the product specific data, defined above:
   *
   *  byte[0-3]: pdh_product_seq_number
   *  byte[4]: psh_product_type
   *  byte[5]: psh_product_category
   *  byte[6]: psh_product_code
   *  byte[7]: np_channel_index
   *  byte[8]: start of fname
   *  byte[8 + FNAME_SIZE + 1]: start of data (fpath in this case)
   */

  packetinfo->seq_number = unpack_uint32(p, 0);
  packetinfo->psh_product_type = p[4];
  packetinfo->psh_product_category = p[5];
  packetinfo->psh_product_code = p[6];
  packetinfo->np_channel_index = p[7];

  p += 8;
  packetinfo->fname = p;

  p += FNAME_SIZE + 1;
  packetinfo->fpath = p;

  return(status);
}

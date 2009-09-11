/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <err.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <ctype.h>
#include "const.h"
#include "readn.h"
#include "util.h"
#include "file.h"
#include "nbs1.h"

/*
 * Functions to support the transmission of the entire files (NBS1).
 * The functions for NBS2 (fpath) are in packfp.{h,c}. This file
 * contains the "receive" functions, which are not used by the server.
 * The "send" functions (which are used by the server) are in nbs1s.c.
 */

int unpack_nbs1_header(struct nbs1_packet_st *nbs){
  /*
   * This function is not used by the server, but it is here as
   * reference for clients.
   */
  unsigned char *p;
  uint32_t dataid;

  /*
   * This function assumes that a read() of at least
   * NBS_ENVELOPE_SIZE + NBS1_HEADER_SIZE bytes
   * has been made and stored in nbs->packet[]. 
   */

  p = (unsigned char*)nbs->packet;

  /*
   * The first 12 bytes are the envelope [nbs_pack_envelope() in packfp.c]
   * We can check that we are getting the correct id, checksum and size.
   */
  dataid = unpack_uint32(p, 0);
  /* assert(dataid == PACKID_FDATA); */
  if(dataid != PACKID_FDATA)
    return(1);

  p += NBS_ENVELOPE_SIZE;
  nbs->seq_number = unpack_uint32(p, 0);
  nbs->psh_product_type = p[4];
  nbs->psh_product_category = p[5];
  nbs->psh_product_code = p[6];
  nbs->np_channel_index = p[7];
  p += 8;
  strncpy(nbs->fbasename, (char*)p, FBASENAME_SIZE + 1);
  p += FBASENAME_SIZE + 1;
  nbs->f_zip = p[0];
  nbs->num_blocks = unpack_uint16(p, 1);  
  nbs->block_number = unpack_uint16(p, 3);  
  nbs->block_size = unpack_uint32(p, 5);
  nbs->packet_size = NBS_ENVELOPE_SIZE + NBS1_HEADER_SIZE + nbs->block_size;

  return(0);
}

int recv_nbs_packet(int fd, struct nbs1_packet_st *nbs,
		    unsigned int timeout_s, int retry){
  /*
   * Returns:
   *  0 => no errors
   * -1 => read error
   * -2 => timed out (poll) before reading anything
   * -3 => disconnected (eof) before reading anything
   *  1 => short read (time out or disconnect) while reading 
   *  2 => checksum error (corrupt packet) or incorrect type of packet.
   *
   * In we need or want to disnguish whether it is a disconnect or time out
   * in the case of the short read we can use sreadn() instead of readn()
   * below.
   */
  int status = 0;
  ssize_t n = 0;
  unsigned char *p;
  uint32_t data_size;
  uint32_t dataid;
  /* uint32_t transmitted_checksum, calculated_checksum; */

  n = readn(fd, nbs->packet, NBS_ENVELOPE_SIZE, timeout_s, retry);
  if(n == -1)
    status = -1;
  else if(n == -2)
    status = -2;
  else if(n == 0)
    status = -3;
  else if(n != NBS_ENVELOPE_SIZE)
    status = 1;

  if(status != 0)
    return(status);

  p = (unsigned char*)nbs->packet;

  /*
   * The first 12 bytes are the envelope [nbs_pack_envelope() in packfp.c]
   */
  dataid = unpack_uint32(p, 0);
  /* assert(dataid == PACKID_FDATA); */
  if(dataid != PACKID_FDATA)
    return(1);

  data_size = unpack_uint32(p, 4);
  /* transmitted_checksum = unpack_uint32(p, 8); */

  /* assert(data_size + NBS_ENVELOPE_SIZE <= NBS1_PACKET_MAXSIZE); */
  if(data_size + NBS_ENVELOPE_SIZE > NBS1_PACKET_MAXSIZE)
    return(1);

  n = readn(fd, &nbs->packet[NBS_ENVELOPE_SIZE], data_size, timeout_s, retry);
  if(n == -1)
    status = -1;
  else if(n == -2)
    status = -2;
  else if(n == 0)
    status = -3;
  else if((size_t)n != data_size)
    status = 1;

  if(status != 0)
    return(status);

  status = unpack_nbs1_header(nbs);
  if(status != 0)
    return(2);

  /*
   * nbs->block points to the start of the file block data.
   */
  nbs->block = nbs->packet;
  nbs->block += NBS_ENVELOPE_SIZE + NBS1_HEADER_SIZE;

  /*
   * Verify the checksum, etc.
   */
  if(data_size != NBS1_HEADER_SIZE + nbs->block_size)
    return(2);

  /*
   * There should be no need to check this if nbsp is a tcp slave.
   * It could be needed if other method is used.
   * Tue May  5 20:28:13 AST 2009
   *
  p += NBS_ENVELOPE_SIZE;
  calculated_checksum = calc_checksum(p, data_size);
  if(calculated_checksum != transmitted_checksum)
    return(2);
  */

  return(status);
}

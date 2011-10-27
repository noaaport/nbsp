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
#include "readn.h"
#include "packfp.h"
#include "packfpu.h"

#define NBSFP_PINFO_SIZE	(8 + FNAME_SIZE + 1)

/*
 * These are the functions used to pack the data that is sent
 * to the server and filter queues (the fpath of the file, NBS2).
 * (The unpack functions are in packfpu.{h,c}.
 * The functions to support the transmission of the entire files (NBS1)
 * are in nbs1{r,s}.{c,h}.
 */

int nbsfp_pack_fpath(struct packet_info_st *packetinfo,
		   struct pctl_element_st *pce){
  /*
   * The format of the data[] sent to the server and filter queues:
   *
   *  byte[0-3]: pdh_product_seq_number
   *  byte[4]: psh_product_type
   *  byte[5]: psh_product_category
   *  byte[6]: psh_product_code
   *  byte[7]: np_channel_index (0-3)
   *  byte[8-(8+FNAME_SIZE)]: fname (including final '\0')
   *  full path of file (including the trailing '\0' and padding to
   *                     fit the db queue record length if necessary)
   *
   *  This is preceeded by the envelope (see below).
   *
   *  The fpath is just a reference to the starting point of the fpath
   *  inside the packet and not a separate malloced storage.
   *
   *  It is assumed that the calling function has allocated space
   *  of the precise size, by calling ``nbsfp_packetinfo_init()''.
   */ 
  int status;
  char *data;
  size_t fpath_size;
  size_t data_size;
  size_t packet_size;
  char *packetp;

  fpath_size = pce->fpath_size;	/* allocated size of pce->fpath[] */
  data_size = NBSFP_PINFO_SIZE + fpath_size;
  packet_size = data_size + NBS_ENVELOPE_SIZE;

  assert(packetinfo->packet != NULL);
  assert(packetinfo->packet_size == packet_size);

  if((packetinfo->packet == NULL) || 
     (packetinfo->packet_size != packet_size)){
    errno = EINVAL;
    return(-1);
  }

  packetp = (char*)packetinfo->packet;
  data = &(packetp[NBS_ENVELOPE_SIZE]);
  packetinfo->fpath = &(data[NBSFP_PINFO_SIZE]);

  packetinfo->seq_number = pce->seq_number;
  packetinfo->psh_product_type = pce->psh_product_type;
  packetinfo->psh_product_category = pce->psh_product_category;
  packetinfo->psh_product_code = pce->psh_product_code;
  packetinfo->np_channel_index = pce->np_channel_index;

  pack_uint32(data, pce->seq_number, 0);
  data[4] = pce->psh_product_type;
  data[5] = pce->psh_product_category;
  data[6] = pce->psh_product_code;
  data[7] = pce->np_channel_index;
  strncpy(&data[8], pce->fname, FNAME_SIZE + 1);
  strncpy(&data[NBSFP_PINFO_SIZE], pce->fpath, fpath_size);

  status = nbs_pack_envelope(packetinfo->packet, PACKID_FPATH, data_size);

  return(status);
}

int const_fpath_packet_maxsize(void){
  /*
   * Returns (computes) the maximum value of a packet, given the specification,
   * and using the function that calculates the maximum value of an fpath.
   * This valus is needed in particular by the function that initalizes
   * the queue db, which uses fixed length (db Queue type) records.
   */
  int maxsize;

  maxsize = const_fpath_maxsize() + 1 + NBSFP_PINFO_SIZE + NBS_ENVELOPE_SIZE;

  return(maxsize);
}

int nbsfp_packetinfo_init(struct packet_info_st *packetinfo){

  int size;

  size = const_fpath_packet_maxsize();
  packetinfo->packet = malloc(size);
  if(packetinfo->packet == NULL)
    return(-1);

  packetinfo->packet_size = size;
  
  return(0);
}

void nbsfp_packetinfo_cleanup(struct packet_info_st *packetinfo){

  if(packetinfo->packet != NULL)
    free(packetinfo->packet);

  packetinfo->packet = NULL;
  packetinfo->packet_size = 0;
}

int nbsfp_packetinfo_create(struct packet_info_st **packetinfo){

  struct packet_info_st *p;

  p = malloc(sizeof(struct packet_info_st));
  if(p == NULL)
    return(-1);

  if(nbsfp_packetinfo_init(p) != 0){
    free(p);
    return(-1);
  }

  *packetinfo = p;

  return(0);
}

void nbsfp_packetinfo_destroy(struct packet_info_st *packetinfo){

  nbsfp_packetinfo_cleanup(packetinfo);
  free(packetinfo);
}

/*
 * This function is not used by the server. It is used by the slave
 * nbs2 (fpath) clients.
 */
int recv_fp_packet(int fd, struct packet_info_st *packetinfo,
		   unsigned int timeout_s, int retry){
  /*
   * Returns:
   * -1 => read error
   *  0 -> no errors
   *  1 => could not read all (timed out).  
   *  2 => checksum error (corrupt packet) or incorrect type of packet.
   */
  int status = 0;
  ssize_t n = 0;
  unsigned char *p;
  uint32_t data_size;
  uint32_t dataid;
  /* uint32_t transmitted_checksum; */

  n = readn(fd, packetinfo->packet, NBS_ENVELOPE_SIZE, timeout_s, retry);
  if(n == -1)
    status = -1;
  else if(n != NBS_ENVELOPE_SIZE)
    status = 1;

  if(status != 0)
    return(status);

  p = (unsigned char*)packetinfo->packet;

  /*
   * The first 12 bytes are the envelope [nbs_pack_envelope() in pack.c]
   */
  dataid = unpack_uint32(p, 0);
  /* assert(dataid == PACKID_FPATH); */
  if(dataid != PACKID_FPATH)
    return(1);

  data_size = unpack_uint32(p, 4);
  /* transmitted_checksum = unpack_uint32(p, 8); */

  /* assert(data_size + NBS_ENVELOPE_SIZE <= packetinfo->packet_size); */
  if(data_size + NBS_ENVELOPE_SIZE > packetinfo->packet_size)
    return(1);

  p += NBS_ENVELOPE_SIZE;
  n = readn(fd, p, data_size, timeout_s, retry);
  if(n == -1)
    status = -1;
  else if((n < 0) || ((size_t)n != data_size))
    status = 1;

  if(status != 0)
    return(status);

  status = nbsfp_unpack_fpath(packetinfo);
  if(status != 0)
    return(2);

  return(status);
}

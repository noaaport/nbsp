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
#include "pack.h"

int nbs_pack_envelope(void *packet, unsigned int dataid, size_t data_size){
  /*
   * The envelope of the data packet contains:
   *
   * 4 bytes for the data id
   * 4 bytes for the size
   * 4 bytes for the checksum of the data[]
   * the data[] (see the function above for the case of fpath or nbs.c for
   * the case of the file content.)
   *
   * This function assumes that all the packet data is already
   * stored in &packet[NBS_ENVELOPE_SIZE] and then it fills the envelope.
   */
  unsigned char *p;
  unsigned char *data;
  uint32_t checksum = 0;

  p = (unsigned char*)packet;
  data = &p[NBS_ENVELOPE_SIZE];

  pack_uint32(p, dataid, 0);
  pack_uint32(p, data_size, 4);
  checksum = calc_checksum(data, data_size);
  pack_uint32(p, checksum, 8);

  return(0);
}

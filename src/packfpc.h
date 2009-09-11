/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef PACKFPC_H
#define PACKFPC_H

/* #include <stdlib.h> */
#include <stdint.h>
#include "const.h"
#include "pack.h"

/*
 * The packfpx.y files contain the functions used to pack/unpack
 * the data that is to the server and filter queues
 * (the fpath of the file). This file contain common definitions.
 * for
 */

#define NBSFP_PINFO_SIZE	(8 + FNAME_SIZE + 1)

/*
 * This data is extracted from the pce and is used for transmission.
 * The fname and fpath are not independent pointers; they point to
 * their starting character inside the *packet.
 */
struct packet_info_st {
  uint32_t seq_number;
  int psh_product_type;
  int psh_product_category;
  int psh_product_code;
  int np_channel_index;
  char *fname;		/* '\0' terminated */
  char *fpath;		/* '\0' terminated */
  void *packet;		/* packed data (for server, fifos and NBS2 clients) */
  size_t packet_size;	/* size of packed data */
};

/*
 * These functions are used by the filter and server to allocate
 * a static memory pool for the packets retrieved from the queue
 * for processing. In this way they avoid having the qdb_rcv function
 * allocate space everytime.
 */
int nbsfp_packetinfo_init_pool(struct packet_info_st *packetinfo);
void nbsfp_packetinfo_destroy_pool(struct packet_info_st *packetinfo);

#endif

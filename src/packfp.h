/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef PACKFP_H
#define PACKFP_H

/*
 * The functions used to pack and unpack the data that is stored in
 * the server and filter queues (the fpath of the file) are defined in the
 * files packfp.{h,c} and packfpu.{h,c}, respectively. They have been
 * divided because the nbspqdc utility uses the unpacking function.
 */
#include "pctl.h"
#include "packfpc.h"

int nbsfp_pack_fpath(struct packet_info_st *packetinfo,
		     struct pctl_element_st *pce);

/*
 * This function is not used by the server. It is used by the slave
 * nbs2 (fpath) clients.
 */
int recv_fp_packet(int fd, struct packet_info_st *packetinfo,
		   unsigned int timeout_s, int retry);

#endif

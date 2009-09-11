/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef PACKFPU_H
#define PACKFPU_H

#include "packfpc.h"

/*
 * This is the function used to unpack the data that is stored in
 * the server and filter queues (the fpath of the file).
 */
int nbsfp_unpack_fpath(struct packet_info_st *packetinfo);

#endif

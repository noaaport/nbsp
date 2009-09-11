/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef PACK_H
#define PACK_H


#define NBS_ENVELOPE_SIZE	12

/* The exported data is the file content (used in nbs.c) */
#define PACKID_FDATA	1

/* 
 * The exported data is the (full) file name. This is what all the pack
 * functions in packfp.c use.
 */
#define PACKID_FPATH	2

int nbs_pack_envelope(void *packet, unsigned int dataid, size_t data_size);

#endif

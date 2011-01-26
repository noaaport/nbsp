/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCNIDS_DECODE_H
#define DCNIDS_DECODE_H

#include "dcnids_header.h"
#include "dcnids.h"

#define NIDS_DBF_CODENAME	"code"  /* parameter name in dbf file */
#define NIDS_DBF_LEVELNAME	"level" /* parameter name in dbf file */

#define NIDS_PDB_MODE_MAINTENANCE	0
#define NIDS_PDB_MODE_CLEAR		1
#define NIDS_PDB_MODE_PRECIPITATION	2

/* packet types */
#define NIDS_PACKET_RADIALS_AF1F		0xaf1f
#define NIDS_PACKET_DIGITAL_RADIALS_16		16

/* product types */
#define NIDS_PDB_CODE_NXR		19
#define NIDS_PDB_CODE_N0Z		20
#define NIDS_PDB_CODE_NXQ		94
#define NIDS_PDB_CODE_NXV		27
#define NIDS_PDB_CODE_NXU		99

/* default filter min and max level values if the filtering option is set */
#define NIDS_BREF_LEVEL_MIN_VAL	1
#define NIDS_BREF_LEVEL_MAX_VAL	96
#define NIDS_RVEL_LEVEL_MIN_VAL	-64
#define NIDS_RVEL_LEVEL_MAX_VAL	64

/* These are arbitrary values that we assign to the rvel codes 0 and 15 */
#define NIDS_RVEL_LEVEL_ND_MIN	-65
#define NIDS_RVEL_LEVEL_ND_MAX	65

/*
 * When decoding the data packets, we must go to the start of the
 * "individual radials". In bytes:
 * symbologoy block => 10 
 * symbology layer =>  6
 * radial packet header => 14
 */
#define NIDS_PACKET_RADIALS_START_RUNS		30

struct nids_product_symbol_block_st {
  int blockid;		/* should be 1 */
  unsigned int blocklength;
  int numlayers;
  unsigned int psb_layer_blocklength;
};

struct nids_radial_packet_header_st {
  int packet_code;
  int first_bin_index;
  int numbins;
  int center_i;
  int center_j;
  int scale;
  int numradials;
};

struct nids_data_st {
  unsigned char *data;		/* file data excluding 120 byte header*/
  unsigned int data_size;	/* "msg size" - nids header (120) */
  struct nids_header_st nids_header;
  struct nids_product_symbol_block_st psb;
  struct nids_radial_packet_header_st radial_packet_header;
  struct dcnids_polygon_map_st polygon_map;
};

void nids_decode_radials_af1f(struct nids_data_st *nd);
void nids_decode_radials_af1f_grided(struct nids_data_st *nd);
void nids_decode_digital_radials_16(struct nids_data_st *nd);

#endif

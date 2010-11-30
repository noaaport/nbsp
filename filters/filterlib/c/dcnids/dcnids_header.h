/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCNIDS_HEADER_H
#define DCNIDS_HEADER_H

#include <time.h>

#define NIDS_HEADER_SIZE	120	/* message and pdb */

struct nids_header_st {
  unsigned char header[NIDS_HEADER_SIZE];
  int m_code;
  int m_days;
  unsigned int m_seconds;
  unsigned int m_msglength;	/* file length without header or trailer */
  int m_source;                 /* unused */
  int m_destination;            /* unused */
  int m_numblocks;              /* unused */
  int pdb_lat;
  int pdb_lon;
  int pdb_height;
  int pdb_code;
  int pdb_mode;
  int pdb_version;
  unsigned int pdb_symbol_block_offset;
  unsigned int pdb_graphic_block_offset;
  unsigned int pdb_tabular_block_offset;
  /* derived values */
  double lat;
  double lon;
  time_t unixseconds;
  int year;
  int month;
  int day;
  int hour;
  int min;
  int sec;
};

void dcnids_decode_header(struct nids_header_st *nheader);

#endif

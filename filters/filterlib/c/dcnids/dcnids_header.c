/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <time.h>
#include <ctype.h>
#include "util.h"
#include "const.h"
#include "dcnids_extract.h"
#include "dcnids_header.h"

void dcnids_decode_header(struct nids_header_st *nheader){

  unsigned char *b;
  int n;
  struct tm tm;

  /* Go past the wmo header and to the start of the awips line */
  b = nheader->buffer;
  b += CTRLHDR_WMO_SIZE;
  
  for(n = 0; n < WMO_AWIPS_SIZE; ++n)
    nheader->awipsid[n] = tolower(*b++);

  nheader->awipsid[WMO_AWIPS_SIZE] = '\0';

  /* Go past the the wmo and awips headers */
  b = nheader->buffer;
  b += WMOAWIPS_HEADER_SIZE;

  nheader->m_code = extract_uint16(b, 1);
  nheader->m_days = extract_uint16(b, 2) - 1;
  nheader->m_seconds = extract_uint32(b, 3);

  /* msglength is the file length without headers or trailers */
  nheader->m_msglength = extract_uint32(b, 5); 
  nheader->m_source = extract_uint16(b, 7);
  nheader->m_destination = extract_uint16(b, 8);
  nheader->m_numblocks = extract_uint16(b, 9);

  nheader->pdb_lat = extract_int32(b, 11);
  nheader->pdb_lon = extract_int32(b, 13);

  nheader->pdb_height = extract_uint16(b, 15);
  nheader->pdb_code = extract_uint16(b, 16);    /* same as m_code */
  nheader->pdb_mode = extract_uint16(b, 17);

  nheader->pdb_version = extract_uint8(b, 54);
  nheader->pdb_symbol_block_offset = extract_uint32(b, 55) * 2;
  nheader->pdb_graphic_block_offset = extract_uint32(b, 57) * 2;
  nheader->pdb_tabular_block_offset = extract_uint32(b, 59) * 2;

  /* derived */
  nheader->lat = ((double)nheader->pdb_lat)/1000.0;
  nheader->lon = ((double)nheader->pdb_lon)/1000.0;
  nheader->unixseconds = nheader->m_days * 24 * 3600 + nheader->m_seconds;

  (void)gmtime_r(&nheader->unixseconds, &tm);
  nheader->year = tm.tm_year + 1900;
  nheader->month = tm.tm_mon + 1;
  nheader->day = tm.tm_mday;
  nheader->hour = tm.tm_hour;
  nheader->min = tm.tm_min;
  nheader->sec = tm.tm_sec;
}

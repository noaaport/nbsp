/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCGINI_PDB_H
#define DCGINI_PDB_H

#include <time.h>
#include "const.h"

#define NESDIS_WMO_HEADER_SIZE  CTRLHDR_WMO_SIZE	/* common.h */
#define NESDIS_WMOID_SIZE	WMO_ID_SIZE		/* common.h */
#define NESDIS_PDB_SIZE		512
#define NESDIS_DATA_BLOCKSIZE	5120

struct nesdis_pdb {
  char buffer[NESDIS_WMO_HEADER_SIZE + NESDIS_PDB_SIZE];
  int  buffer_size;
  char wmoid[NESDIS_WMOID_SIZE + 1];
  /* From AAO13008.pdf (p. 26) */
  int source;			/* should be 1 => NESDIS */
  int creating_entity;
  int sector;
  int channel;
  int numlines;
  int linesize;
  int year;
  int month;
  int day;
  int hour;
  int min;
  int secs;
  int hsecs;	/* hundredths of second */
  int nx;	/* octet 17, 18 */
  int ny;	/* octet 19, 20 */
  int res;	/* resolution */
  /* extended parameters */
  int map_projection;
  int dx;
  int dy;
  int lat1;
  int lon1;
  int lov;
  int latin;
  int lat2;
  int lon2;
  int lat_ur;
  int lon_ur;
  /*
   * derived
   */
  time_t unixseconds;
  double dx_meters;
  double dy_meters;
  double lat1_deg;
  double lon1_deg;
  double lov_deg;
  double latin_deg;
  double lat2_deg;
  double lon2_deg;
  double lat_ur_deg;
  double lon_ur_deg;
  double lat1_rad;
  double lon1_rad;
  double lov_rad;
  double latin_rad;
  double lat2_rad;
  double lon2_rad;
  double lat_ur_rad;
  double lon_ur_rad;
};

int read_nesdis_pdb(int fd, struct nesdis_pdb *npdb);
int read_nesdis_pdb_compressed(int fd, struct nesdis_pdb *npdb);
void fill_nesdis_pdb(struct nesdis_pdb *npdb);

#endif

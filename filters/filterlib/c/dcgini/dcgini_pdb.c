/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <zlib.h>
#include "unz.h"
#include "dcgini_util.h"
#include "dcgini_misc.h"
#include "dcgini_pdb.h"

#define RAD_PER_DEG     0.0174          /* pi/180 */

int read_nesdis_pdb(int fd, struct nesdis_pdb *npdb){
  /*
   * Returns:
   *
   *  0 => ok
   * -1 => read error
   *  1 => corrupt file (short)
   *  2 => invalid wmo header
   */
  int n;

  n = read(fd, npdb->buffer, npdb->buffer_size);
  if(n == -1)
    return(-1);
  else if(n != npdb->buffer_size)
    return(1);

  if(dcgini_verify_wmo_header(npdb->buffer) != 0)
    return(2);

  fill_nesdis_pdb(npdb);

  return(0);
}

int read_nesdis_pdb_compressed(int fd, struct nesdis_pdb *npdb){
  /*
   * This function extracts the information from the nesdis pdb
   * that is used by the filters. It tries to
   * detect if the input file is compresed (in the gempak-like form
   * with a wmo and the compressed frames appended one after the other).
   * If the file is compressed, then there is a plain text wmo. Then 
   * comes the first compressed frame, which contains the wmo
   * and the pdb after it is uncompressed.
   *
   * returns:
   *
   *  0 => ok
   * -1 => read error
   *  1 => file too short
   *  2 => Invalid wmo header
   *  3 => zlib error
   */
  int n;
  int i;
  unsigned char buffer[NESDIS_WMO_HEADER_SIZE + NESDIS_PDB_SIZE];
  int buffer_size = NESDIS_WMO_HEADER_SIZE + NESDIS_PDB_SIZE;
  unsigned char *p;
  int status = 0;

  /* Initialize */
  npdb->buffer_size = NESDIS_WMO_HEADER_SIZE + NESDIS_PDB_SIZE;

  n = read(fd, buffer, buffer_size);
  if(n == -1)
    return(-1);

  p = &buffer[NESDIS_WMO_HEADER_SIZE];
  if((p[0] == ZBYTE0) && (p[1] == ZBYTE1)){
    /*
     * Must decompress. Find the end of the compressed frame.
     */
    n -= NESDIS_WMO_HEADER_SIZE;  /* number of bytes in buffer after wmo */
    i = 0;
    status = Z_DATA_ERROR;
    while(i < n){
      ++i;
      if((p[i] == ZBYTE0) && (p[i + 1] == ZBYTE1)){
	/*
	 * This should be the start of the next frame. Try to decompress
	 * what we have. If we get an error, try appending more data
	 * until the next marker and keep trying until we get to the end
	 * of the available data.
	 */
	status = unz(npdb->buffer, &npdb->buffer_size, (char*)p, i);
	if(status == Z_OK)
	  break;
      }
    }

    if(status != Z_OK){
      return(3);	/* zlib error */
    }
  }else{
    /*
     * The frame is uncompressed.
     */
    if(n != npdb->buffer_size){
      return(1);	/* file too short */
    }else
      memcpy(npdb->buffer, buffer, n);
  }

  if(dcgini_verify_wmo_header(npdb->buffer) != 0)
    return(2);

  fill_nesdis_pdb(npdb);

  return(0);
}

void fill_nesdis_pdb(struct nesdis_pdb *npdb){

  unsigned char *p;
  int n;
  struct tm tm;

  for(n = 0; n < NESDIS_WMOID_SIZE; ++n)
    npdb->wmoid[n] = tolower(npdb->buffer[n]);

  npdb->wmoid[NESDIS_WMOID_SIZE] = '\0';

  p = (unsigned char*)npdb->buffer;
  p += NESDIS_WMO_HEADER_SIZE;

  npdb->source = (int)p[0];
  npdb->creating_entity = (int)p[1];
  npdb->sector = (int)p[2];
  npdb->channel = (int)p[3];

  /*
  npdb->numlines = (int)((p[4] << 8) + p[5]);
  npdb->linesize = (int)((p[6] << 8) + p[7]);
  */
  npdb->numlines = (int)dcgini_unpack_uint16(p, 4);
  npdb->linesize = (int)dcgini_unpack_uint16(p, 6);

  npdb->year = (int)p[8];
  npdb->year += 1900;
  npdb->month = (int)p[9];
  npdb->day = (int)p[10];
  npdb->hour = (int)p[11];
  npdb->min = (int)p[12];
  npdb->secs = (int)p[13];
  npdb->hsecs = (int)p[14];

  /*
  npdb->nx = (int)((p[16] << 8) + p[17]);
  npdb->ny = (int)((p[18] << 8) + p[19]);
  */
  npdb->nx = (int)dcgini_unpack_uint16(p, 16);
  npdb->ny = (int)dcgini_unpack_uint16(p, 18);

  npdb->res = (int)p[41];

  /* extended parameters */
  npdb->map_projection = (int)p[15];
  if(npdb->map_projection == 1){
    /*
     * mercator
     */
    npdb->lat1 = dcgini_unpack_int24(p, 20);
    npdb->lon1 = dcgini_unpack_int24(p, 23);
    npdb->lov = 0;	/* not given */
    npdb->dx = (int)dcgini_unpack_uint16(p, 33);
    npdb->dy = (int)dcgini_unpack_uint16(p, 35);
    npdb->latin = dcgini_unpack_int24(p, 38);
    npdb->lat2 = dcgini_unpack_int24(p, 27);
    npdb->lon2 = dcgini_unpack_int24(p, 30);
    npdb->lat_ur = dcgini_unpack_int24(p, 55);
    npdb->lon_ur = dcgini_unpack_int24(p, 58);
  }else{
    /*
     * lambert == 3, polar == 5
     */
    npdb->lat1 = dcgini_unpack_int24(p, 20);
    npdb->lon1 = dcgini_unpack_int24(p, 23);
    npdb->lov = dcgini_unpack_int24(p, 27);
    npdb->dx = (int)dcgini_unpack_uint24(p, 30);
    npdb->dy = (int)dcgini_unpack_uint24(p, 33);
    npdb->latin = dcgini_unpack_int24(p, 38);
    npdb->lat2 = 0;	/* not given */
    npdb->lon2 = 0;	/* not given */
    npdb->lat_ur = dcgini_unpack_int24(p, 55);
    npdb->lon_ur = dcgini_unpack_int24(p, 58);
  }

  /*
  npdb->scan_mode = (int)p[37];
  */

  /*
   * derived
   */
  tm.tm_sec = npdb->secs;
  tm.tm_min = npdb->min;
  tm.tm_hour = npdb->hour;
  tm.tm_mday = npdb->day;
  tm.tm_mon = npdb->month - 1;
  tm.tm_year = npdb->year - 1900;
  npdb->unixseconds = timegm(&tm);

  npdb->dx_meters = npdb->dx_meters/10.0;
  npdb->dy_meters = npdb->dy_meters/10.0;

  /*
   * set klist [list lat1 lon1 lov latin lat2 lon2 lat_ur lon_ur];
   * foreach k $klist {
   *   puts "npdb->${k}_deg = npdb->${k}/10000.0;";
   *   puts "npdb->${k}_rad = npdb->${k}_deg * RAD_PER_DEG;";
   * }
   */
  npdb->lat1_deg = npdb->lat1/10000.0;
  npdb->lat1_rad = npdb->lat1_deg * RAD_PER_DEG;
  npdb->lon1_deg = npdb->lon1/10000.0;
  npdb->lon1_rad = npdb->lon1_deg * RAD_PER_DEG;
  npdb->lov_deg = npdb->lov/10000.0;
  npdb->lov_rad = npdb->lov_deg * RAD_PER_DEG;
  npdb->latin_deg = npdb->latin/10000.0;
  npdb->latin_rad = npdb->latin_deg * RAD_PER_DEG;
  npdb->lat2_deg = npdb->lat2/10000.0;
  npdb->lat2_rad = npdb->lat2_deg * RAD_PER_DEG;
  npdb->lon2_deg = npdb->lon2/10000.0;
  npdb->lon2_rad = npdb->lon2_deg * RAD_PER_DEG;
  npdb->lat_ur_deg = npdb->lat_ur/10000.0;
  npdb->lat_ur_rad = npdb->lat_ur_deg * RAD_PER_DEG;
  npdb->lon_ur_deg = npdb->lon_ur/10000.0;
  npdb->lon_ur_rad = npdb->lon_ur_deg * RAD_PER_DEG;
}

/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>
#include <zlib.h>
#include <libgen.h>
#include "err.h"
#include "unz.h"
#include "const.h"

/*
 * Usage:  nbspsatinfo [-i <fpath>] < <stdin>
 *
 * If no file is given in the argument, then it reads from stdin. It
 * can take either the compressed or uncompresed file as input,
 * and it extract the relevant info to stdout.
 * The functionality is similar to nbspsat -i, but the output
 * does not contain the fname:
 *
 *         npdb->source, 
 *         npdb->creating_entity, 
 *         npdb->sector, 
 *         npdb->channel,
 *         npdb->res,
 *         (uintmax_t)time
 *
 * With the [-e] option it prints the following additional values after those:
 *
 *	lat1  (lat of first grid point) int x 10000
 *	lon1  (lon of first grid point) int x 10000
 *      lov   (orientation of grid)     int x 10000
 *      dx    (x-direction increment)
 *      dy    (y-direction increment)
 *      latin (Earth tangent)           int x 10000
 *      lat2  (upper right-hand corner lat) int x 10000
 *      lon2  (upper right-hand corner lon) int x 10000
 */
#define WMO_HEADER_SIZE		CTRLHDR_WMO_SIZE	/* common.h */
#define WMOID_SIZE		WMO_ID_SIZE		/* common.h */
#define NESDIS_PDB_SIZE		512
#define NESDIS_DATA_BLOCKSIZE	5120

struct nesdis_product_def_block {
  char buffer[WMO_HEADER_SIZE + NESDIS_PDB_SIZE];
  char wmoid[WMOID_SIZE + 1];
  int  buffer_size;
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
  int nx;
  int ny;
  int res;	/* resolution */
  /* extended parameters */
  int lat1;
  int lon1;
  int lov;
  int dx;
  int dy;
  int latin;
  int lat2;
  int lon2;
};

struct {
  int opt_background;
  int opt_extended_info;
  char *opt_inputfile;
} g = {0, 0, NULL};

int read_nesdis_pdb(int fd, struct nesdis_product_def_block *npdb);
void fill_nesdis_pdb(struct nesdis_product_def_block *npdb);
int write_file_info(char *in_file);
void output(struct nesdis_product_def_block *npdb, int opt_extended_info);

static unsigned int unpack_uint16(void *p, size_t start);
static unsigned int unpack_uint24(void *p, size_t start);

int main(int argc, char **argv){

  int status = 0;
  int c;
  char *optstr = "behi:";
  char *usage = "nbspsatinfo [-b] [-e] [-h] [-i file]";

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'e':
      g.opt_extended_info = 1;
      break;      
    case 'i':
      g.opt_inputfile = optarg;
      break;
    case 'h':
    default:
      log_info(usage);
      exit(0);
      break;
    }
  }

  if(g.opt_background == 1)
    set_usesyslog();

  status = write_file_info(g.opt_inputfile);

  return(status != 0 ? 1 : 0);
}

int read_nesdis_pdb(int fd, struct nesdis_product_def_block *npdb){

  int n;

  n = read(fd, npdb->buffer, npdb->buffer_size);
  if(n == -1)
    return(-1);
  else if(n != npdb->buffer_size)
    return(1);

  fill_nesdis_pdb(npdb);

  return(0);
}

void fill_nesdis_pdb(struct nesdis_product_def_block *npdb){

  unsigned char *p;
  int n;

  p = (unsigned char*)npdb->buffer;
  p += WMO_HEADER_SIZE;

  npdb->source = (int)p[0];
  npdb->creating_entity = (int)p[1];
  npdb->sector = (int)p[2];
  npdb->channel = (int)p[3];

  /*
  npdb->numlines = (int)((p[4] << 8) + p[5]);
  npdb->linesize = (int)((p[6] << 8) + p[7]);
  */
  npdb->numlines = (int)unpack_uint16(p, 4);
  npdb->linesize = (int)unpack_uint16(p, 4);

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
  npdb->nx = (int)unpack_uint16(p, 16);
  npdb->ny = (int)unpack_uint16(p, 18);

  npdb->res = (int)p[41];

  /* extended parameters */
  npdb->lat1 = (int)unpack_uint24(p, 20);
  npdb->lon1 = (int)unpack_uint24(p, 23);
  npdb->lov = (int)unpack_uint24(p, 27);
  npdb->dx = (int)unpack_uint24(p, 30);
  npdb->dy = (int)unpack_uint24(p, 33);
  npdb->latin = (int)unpack_uint24(p, 38);
  npdb->lat2 = (int)unpack_uint24(p, 55);
  npdb->lon2 = (int)unpack_uint24(p, 58);

  for(n = 0; n <= WMOID_SIZE - 1; ++n)
    npdb->wmoid[n] = tolower(npdb->buffer[n]);

  npdb->wmoid[WMOID_SIZE] = '\0';
}

int write_file_info(char *in_file){
  /*
   * This function extracts and prints (to stdout) the information
   * from the nesdis pdb that is used by the filters. It tries to
   * detect if the input file is compresed (in the gempak-like form
   * with a wmo and the compressed frames appended one after the other.
   * If the file is compressed, then there is a plain text wmo. Then 
   * comes the first compressed frame, which contains the wmo
   * and the pdb after it is uncompressed.
   */
  int fd;
  int n;
  int i;
  struct nesdis_product_def_block npdb;
  char buffer[WMO_HEADER_SIZE + NESDIS_PDB_SIZE];
  int buffer_size = WMO_HEADER_SIZE + NESDIS_PDB_SIZE;
  unsigned char *p;
  int status = 0;

  /* Initialize */
  npdb.buffer_size = WMO_HEADER_SIZE + NESDIS_PDB_SIZE;

  if(in_file != NULL){
    fd = open(in_file, O_RDONLY);
    if(fd == -1){
      log_err(1, "Could not open %s", in_file);
      return(-1);
    }
  }else
    fd = fileno(stdin);

  n = read(fd, buffer, buffer_size);
  if(in_file != NULL){
    close(fd);
  }

  if(n == -1){
    if(in_file != NULL)
      log_err(1, "Error reading from %s", in_file);
    else
      log_err(1, "Error reading from stdin.");

    return(-1);
  }

  p = (unsigned char*)&buffer[WMO_HEADER_SIZE];
  if((p[0] == ZBYTE0) && (p[1] == ZBYTE1)){
    /*
     * Must decompress. Find the end of the compressed frame.
     */
    n -= WMO_HEADER_SIZE;	/* number of bytes in buffer after wmo */
    i = 0;
    status = Z_DATA_ERROR;
    while(i <= n - 1){
      ++i;
      if((p[i] == ZBYTE0) && (p[i + 1] == ZBYTE1)){
	/*
	 * This should be the start of the next frame. Try to decompress
	 * what we have. If we get an error, try appending more data
	 * until the next marker and keep trying until we get to the end
	 * of the available data.
	 */
	status = unz(npdb.buffer, &npdb.buffer_size, (char*)p, i);
	if(status == Z_OK)
	  break;
      }
    }

    if(status != Z_OK){
      log_errx(1, "Could not read nesdis pdb.");
      return(1);
    }
  }else{
    /*
     * The frame is uncompressed.
     */
    if(n != npdb.buffer_size){
      log_errx(1, "Error reading pdb. File too short.");
      return(-1);
    }else
      memcpy(npdb.buffer, buffer, n);
  }

  fill_nesdis_pdb(&npdb);

  output(&npdb, g.opt_extended_info);

  return(0);
}

void output(struct nesdis_product_def_block *npdb, int opt_extended_info){

  struct tm tm;
  time_t time;

  tm.tm_sec = npdb->secs;
  tm.tm_min = npdb->min;
  tm.tm_hour = npdb->hour;
  tm.tm_mday = npdb->day;
  tm.tm_mon = npdb->month - 1;
  tm.tm_year = npdb->year - 1900;

  time = timegm(&tm);

  fprintf(stdout, "%d %d %d %d %d " "%" PRIuMAX, 
	  npdb->source, 
	  npdb->creating_entity, 
	  npdb->sector, 
	  npdb->channel,
	  npdb->res,
	  (uintmax_t)time);

  if(opt_extended_info == 0){
    fprintf(stdout, "\n");
    return;
  }

  fprintf(stdout, " %d %d %d %d %d %d %d %d\n",
	  npdb->lat1,
	  npdb->lon1,
	  npdb->lov,
	  npdb->dx,
	  npdb->dy,
	  npdb->latin,
	  npdb->lat2,
	  npdb->lon2);
}

/*
 * utility functions
 */
static unsigned int unpack_uint16(void *p, size_t start){
  /*
   * The first byte is the most significant one and the last byte is
   * the least significant.
   */
  unsigned int u;
  unsigned char *uptr;

  uptr = &((unsigned char*)p)[start];

  u = (uptr[0] << 8) + uptr[1];

  return(u);
}

static unsigned int unpack_uint24(void *p, size_t start){
  /*
   * The first byte is the most significant one and the last byte is
   * the least significant.
   */
  unsigned int u;
  unsigned char *uptr;

  uptr = &((unsigned char*)p)[start];

  u = (uptr[0] << 16) + (uptr[1] << 8) + uptr[2];

  return(u);
}

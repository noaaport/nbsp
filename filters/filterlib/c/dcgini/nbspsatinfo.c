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
#include <libgen.h>
#include "err.h"
#include "dcgini_pdb.h"
#include "dcgini_util.h"
#include "dcgini_name.h"

/*
 * Usage:  nbspsatinfo [-i <fpath>] < <stdin>
 *
 * If no file is given in the argument, then it reads from stdin. It
 * can take either the compressed or uncompresed file as input,
 * and it extract the relevant info to stdout.
 * The functionality is similar to nbspsat -i, but the output
 * does not contain the fname and the time is the unixseconds:
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
 *	map_projection (map projection indicator)
 *	lat1  (lat of first grid point) int x 10000
 *	lon1  (lon of first grid point) int x 10000
 *      lov   (orientation of grid)     int x 10000
 *      dx    (x-direction increment)
 *      dy    (y-direction increment)
 *	nx    (number of points in x direction)
 *	ny    (number of points in y direction)
 *	res   (same thing as above; duplicated for convenience)
 *      latin (Earth tangent)           int x 10000
 *	lat2  (lat of last grid point) int x 10000
 *	lon2  (lon of last grid point) int x 10000
 *      lat_ur  (upper right-hand corner lat) int x 10000
 *      lon_ur  (upper right-hand corner lon) int x 10000
 */

struct {
  int opt_background;
  int opt_extended_info;
  char *opt_inputfile;
} g = {0, 0, NULL};

int write_file_info(char *in_file);
void output(struct nesdis_pdb *npdb, int opt_extended_info);

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
  struct nesdis_pdb npdb;
  int status = 0;

  if(in_file != NULL){
    fd = open(in_file, O_RDONLY);
    if(fd == -1){
      log_err(1, "Could not open %s", in_file);
      return(-1);
    }
  }else
    fd = fileno(stdin);

  status = read_nesdis_pdb_compressed(fd, &npdb);
  if(in_file != NULL){
    close(fd);
  }

  if(status == -1){
    if(in_file != NULL)
      log_err(1, "Error reading from %s", in_file);
    else
      log_err(1, "Error reading from stdin.");

    return(-1);
  }else{
    if(status == 1)
      log_errx(1, "Error reading pdb. File too short.");
    else
      log_errx(1, "Could not read nesdis pdb. Error from zlib.");

    return(1);
  }

  output(&npdb, g.opt_extended_info);

  return(0);
}

void output(struct nesdis_pdb *npdb, int opt_extended_info){

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

  fprintf(stdout, " %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
	  npdb->map_projection,
	  npdb->lat1,
	  npdb->lon1,
	  npdb->lov,
	  npdb->dx,
	  npdb->dy,
	  npdb->nx,
	  npdb->ny,
	  npdb->res,
	  npdb->latin,
	  npdb->lat2,
	  npdb->lon2,
	  npdb->lat_ur,
	  npdb->lon_ur);
}

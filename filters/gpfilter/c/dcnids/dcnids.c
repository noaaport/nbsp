/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <math.h>
#include "err.h"
#include "util.h"
#include "dcnids.h"

/*
 * Usage: nbspradinfo [-c <count>] [output options] <file> | < <file>
 *
 * The program reads from a file or stdin, but the data must start with the
 * nids header (i.e., the ccb and wmo headers must have been removed;
 * but see below).
 *
 * The typical usage is therefore
 *
 * nbspunz -c 54 ksjt_sdus54-n0rsjt.020131_16452648 | nbspradinfo
 * nbspunz -c 54 n0rsjt_20091002_0007.nids | nbspradinfo
 *  
 * In the first case the data file is one from the spool directory;
 * in the second, the data file is from the digatmos/nexrad directory.
 *
 * If the data does not start with nids header, the [-c <count>] options
 * can be used to instruct the program to ignore the first <count> bytes.
 * So an alternative usage is
 *
 * nbspunz ksjt_sdus54-n0rsjt.020131_16452648 | nbspradinfo -c 54
 *
 * or, if the file is not compressed
 *
 * nbspradinfo -c 54 koun_sdus54-n0qvnx.210224_204899761
 * nbspradinfo -c 41 n0qvnx_20100221_0224.nids
 *               (41 = 30 + gempak header [const.h])
 *
 * If the file does not have the gempak header (as the tmp file used
 * by the rstfilter.lib), then
 *
 * nbspradinfo -c 30 n0qvnx_20100221_0224.tmp
 *
 * The default information printed is
 *
 *          nheader.pdb_lat, nheader.pdb_lon, nheader.pdb_height, seconds,
 *          nheader.pdb_mode, nheader.pdb_code
 *
 * If [-t] is given in which case only the "seconds" is printed.
 * Otherwise anyone of
 *
 *  -v <csv file>
 *  -p <shp file>
 *  -x <shx file>
 *  -f <dbf file>
 *
 * will write the corresponding data file.
 */
#define NIDS_HEADER_SIZE	120	/* message and pdb */
#define NIDS_DBF_CODENAME	"code"  /* parameter name in dbf file */
#define NIDS_DBF_LEVELNAME	"level" /* parameter name in dbf file */

#define NIDS_PDB_MODE_MAINTENANCE	0
#define NIDS_PDB_MODE_CLEAR		1
#define NIDS_PDB_MODE_PRECIPITATION	2

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
  unsigned int unixseconds;
  double lat;
  double lon;
};

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

struct nids_radial_packet_st {
  int num_rle_halfwords;
  int angle_start;	/* in tenth of degree */
  int angle_delta;
  /* derived */
  double angle_start_deg;	/* in degrees */
  double angle_delta_deg;
  /* runs */
};

struct nids_data_st {
  unsigned char *data;		/* file data excluding 120 byte header*/
  unsigned int data_size;	/* "msg size" - nids header (120) */
  struct nids_header_st nids_header;
  struct nids_product_symbol_block_st psb;
  struct nids_radial_packet_header_st radial_packet_header;
  struct dcnids_polygon_map_st polygon_map;
};

struct {
  int opt_background;	/* -b */
  int opt_skipcount;	/* -c <count> => skip the first <count> bytes */
  int opt_timeonly;	/* -t => only extract and print the time (unix secs) */
  char *opt_dbffile;	/* -f => write dbf file */
  char *opt_shpfile;	/* -p => write shp file */
  char *opt_csvfile;	/* -v => write csv file */
  char *opt_shxfile;	/* -x => write shx file */
  char *opt_inputfile;
  /* variables */
  int opt_fpvx;		/* set if anyone of fpvx is given */
  int fd;
} g = {0, 0, 0, NULL, NULL, NULL, NULL, NULL, 0, -1};

/* general functions */
static int process_file(void);
static void cleanup(void);

/* decoding functions */
static void nids_decode_header(struct nids_header_st *nids_header);
static void nids_decode_data(struct nids_data_st *nids_data);

/* output functions */
static void nids_csv_write(struct nids_data_st *nids_data);
static void nids_shp_write(struct nids_data_st *nids_data);
static void nids_dbf_write(struct nids_data_st *nids_data);

/* #define PRINT_TEST */
#ifdef PRINT_TEST
static void test_print(struct nids_data_st *nids_data);
#endif

static void cleanup(void){

  if((g.fd != -1) && (g.opt_inputfile != NULL))
    (void)close(g.fd);
}

int main(int argc, char **argv){

  char *optstr = "bc:f:tp:v:x:";
  char *usage = "nbspradinfo [-b] [-c count] [-f dbffile] "
    "[-t] [-p shpfile] [-v csvfile] [-x shxfile] <file> | < file";
  int status = 0;
  int c;
  int opt_px = 0;	/* -p and -x must be given together */

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'c':
      status = strto_int(optarg, &g.opt_skipcount);
      if((status == 1) || (g.opt_skipcount <= 0)){
	log_errx(1, "Invalid argument to [-c] option.");
      }
      break;
    case 'f':
      g.opt_dbffile = optarg;
      g.opt_fpvx = 1;
      break;
    case 'p':
      g.opt_shpfile = optarg;
      g.opt_fpvx = 1;
      ++opt_px;
      break;
    case 't':
      g.opt_timeonly = 1;
      break;
    case 'v':
      g.opt_csvfile = optarg;
      g.opt_fpvx = 1;
      break;
    case 'x':
      g.opt_shxfile = optarg;
      g.opt_fpvx = 1;
      ++opt_px;
      break;
    default:
      log_info(usage);
      exit(0);
      break;
    }
  }

  if((g.opt_fpvx != 0) && (g.opt_timeonly != 0))
    log_errx(1, "Invalid combination of options: t and fpvx");

  if((opt_px != 0) && (opt_px != 2))
    log_errx(1, "Invalid combination of options p and x.");

  if(g.opt_background == 1)
    set_usesyslog();

  if(optind < argc - 1)
    log_errx(1, "Too many arguments.");
  else if(optind == argc -1)
    g.opt_inputfile = argv[optind++];

  atexit(cleanup);
  status = process_file();

  return(status != 0 ? 1 : 0);
}

int process_file(void){

  int fd;
  int n;
  unsigned char *b;
  struct nids_data_st nids_data;
  char dummy[4096];
  size_t dummy_size = 4096;
  size_t nleft;
  size_t ndummy_read;

  memset(&nids_data, 0, sizeof(struct nids_data_st));

  if(g.opt_inputfile == NULL)
    fd = fileno(stdin);
  else{
    fd = open(g.opt_inputfile, O_RDONLY);
    if(fd == -1)
      log_err_open(g.opt_inputfile);
    else
      g.fd = fd;
  }

  b = &nids_data.nids_header.header[0];

  if(g.opt_skipcount != 0){
    nleft = g.opt_skipcount;
    while(nleft > 0){
      ndummy_read = nleft;
      if((size_t)nleft > dummy_size)
	ndummy_read = dummy_size;

      if(read(fd, dummy, ndummy_read) == -1)
	log_err(1, "Error from read()");

      nleft -= ndummy_read;
    }
  }

  n = read(fd, b, NIDS_HEADER_SIZE);
  if(n == -1)
    log_err(1, "Error from read()");
  else if(n < NIDS_HEADER_SIZE)
    log_errx(1, "Corrupt file.");

  nids_decode_header(&nids_data.nids_header);

#ifdef PRINT_TEST
  test_print(&nids_data);
#endif

  if(g.opt_timeonly != 0){
    fprintf(stdout, "%u", nids_data.nids_header.unixseconds);
  }else if(g.opt_fpvx == 0){
    fprintf(stdout, "%.3f %.3f %d %u %d %d",
	    nids_data.nids_header.lat,
	    nids_data.nids_header.lon,
	    nids_data.nids_header.pdb_height,
	    nids_data.nids_header.unixseconds,
	    nids_data.nids_header.pdb_mode,
	    nids_data.nids_header.pdb_code);
  }

  if(g.opt_fpvx == 0){
    /*
     * If reading from stdin, then consume the input to avoid generating
     * a pipe error in the tcl scripts.
     * nbspunz should be called with the [-n] option.
     */
    if(g.opt_inputfile == NULL){
      while(read(fd, dummy, dummy_size) > 0)
	;
    }
    return(0);
  }
  
  /*
   * Decode the polygon data
   */

  nids_data.data = malloc(nids_data.nids_header.m_msglength);
  if(nids_data.data == NULL)
    log_err(1, "Error from malloc()");
  
  n = nids_data.nids_header.m_msglength - NIDS_HEADER_SIZE;
  if(n <= 0)
    log_errx(1, "Corrupt file.");

  if(read(fd, nids_data.data, n) == -1)
     log_err(1, "Error from read()");     

  nids_data.data_size = n;

  nids_decode_data(&nids_data);

  /* Output data */
  if(g.opt_csvfile != NULL)
    nids_csv_write(&nids_data);

  if((g.opt_shpfile != NULL) && (g.opt_shxfile != NULL))
    nids_shp_write(&nids_data);

  if(g.opt_dbffile != NULL)
    nids_dbf_write(&nids_data);

  return(0);
}

/*
 * decoding functions
 */
static void nids_decode_header(struct nids_header_st *nheader){

  unsigned char *b = nheader->header;

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
}

static void nids_decode_data(struct nids_data_st *nd){

  unsigned char *b = nd->data;
  unsigned char *bsave;		/* used when counting the plygongs */
  struct dcnids_polygon_st *polygon;
  struct nids_radial_packet_st radial_packet;
  int i, j;
  int run_code, run_bins, total_bins;
  double r1, r2, theta1, theta2;
  int numpoints = 0;
  int numpolygons = 0;
  
  nd->psb.blockid = extract_uint16(b, 2);	/* should be 1 */
  nd->psb.blocklength = extract_uint32(b, 3);
  nd->psb.numlayers = extract_uint16(b, 5);
  b += 10;

  /* XXX
  fprintf(stdout, "\npsb: %d %u %d\n",
	  nd->psb.blockid,
	  nd->psb.blocklength,
	  nd->psb.numlayers);
  */

  nd->psb.psb_layer_blocklength = extract_uint32(b, 2);
  b += 6;

  /* XXX
  fprintf(stdout, "psb_layer_blocklength = %u\n",
	  nd->psb.psb_layer_blocklength);
  */

  /* start of display packets */
  nd->radial_packet_header.packet_code = extract_uint16(b, 1);
  nd->radial_packet_header.first_bin_index = extract_int16(b, 2);
  nd->radial_packet_header.numbins = extract_uint16(b, 3);
  nd->radial_packet_header.center_i = extract_int16(b, 4);
  nd->radial_packet_header.center_j = extract_int16(b, 5);
  nd->radial_packet_header.scale = extract_uint16(b, 6);
  nd->radial_packet_header.numradials = extract_uint16(b, 7);
  b += 14;

  /* XXX
  fprintf(stdout, "\n%x %d %d %d %d %d %d\n",
	  nd->radial_packet_header.packet_code,
	  nd->radial_packet_header.first_bin_index,
	  nd->radial_packet_header.numbins,
	  nd->radial_packet_header.center_i,
	  nd->radial_packet_header.center_j,
	  nd->radial_packet_header.scale,
	  nd->radial_packet_header.numradials);
  */

  /*
   * We will run through the radials twice. The first timeis just to
   * count the number of polygons, and the second time to actually
   * fill in the polygons. In other words, we must compute the total
   * number of "run bins".
   */
  bsave = b;
  for(i = 0; i < nd->radial_packet_header.numradials; ++i){
    radial_packet.num_rle_halfwords = extract_uint16(b, 1);
    b += 6;
    b += radial_packet.num_rle_halfwords * 2;
    /*
     * This is an upper limit since the last byte may or may not have data.
     */
    numpolygons += radial_packet.num_rle_halfwords * 2;
  }
  b = bsave;

  /*
   * Allocate space for all the polygons.
   */
  nd->polygon_map.numpolygons = numpolygons;
  nd->polygon_map.polygons = malloc(sizeof(struct dcnids_polygon_st) *
				    nd->polygon_map.numpolygons);
  if(nd->polygon_map.polygons == NULL)
    log_err(1, "Error from malloc()");

  /* XXX 
  fprintf(stdout, "numpolygons = %d\n", nd->polygon_map.numpolygons);
  */

  /*
   * Go through the run bins again, this time to get the polygons.
   */

  /*
   * Initialize the polygon pointer to the start of the polygon array,
   * and count them again to check.
   */
  polygon = nd->polygon_map.polygons;
  numpolygons = 0;

  for(i = 0; i < nd->radial_packet_header.numradials; ++i){
    radial_packet.num_rle_halfwords = extract_uint16(b, 1);
    radial_packet.angle_start = extract_int16(b, 2);
    radial_packet.angle_delta = extract_uint16(b, 3);
    b += 6;

    radial_packet.angle_start_deg = (double)radial_packet.angle_start/10.0;
    radial_packet.angle_delta_deg = (double)radial_packet.angle_delta/10.0;

    /* XXX
    fprintf(stdout, "num_rle_halfwords = %d %f %f\n",
	    radial_packet.num_rle_halfwords,
	    radial_packet.angle_start_deg,
	    radial_packet.angle_delta_deg);
    */

    total_bins = 0;
    for(j = 0; j < radial_packet.num_rle_halfwords * 2; ++j){
      run_code = (b[0] & 0xf);
      run_bins = (b[0] >> 4);
      ++b;      

      /*
       * The last byte may be empty (if there is an odd number of range bins).
       */
      if(run_bins == 0)
	continue;  /* should be the last byte and the loop will break itself */

      numpoints += run_bins;

      /* radius in km */
      r1 = ((double)(total_bins * nd->radial_packet_header.scale))/1000.0;
      total_bins += run_bins;
      r2 = ((double)(total_bins * nd->radial_packet_header.scale))/1000.0;

      /* theta1 and theta2 in degrees */
      theta1 = radial_packet.angle_start_deg;
      theta2 = radial_packet.angle_start_deg + radial_packet.angle_delta_deg;

      /*
       * The reference lat, lon are the site coordinates
       */
      dcnids_define_polygon(nd->nids_header.lon,
			    nd->nids_header.lat,
			    r1, r2, theta1, theta2, polygon);
      /*
       * The "level" that corresponds to the code depends on the operational
       * mode.
       */
      polygon->code = run_code;
      if(nd->nids_header.pdb_mode == NIDS_PDB_MODE_PRECIPITATION)
	polygon->level = run_code * 5;
      else if(nd->nids_header.pdb_mode == NIDS_PDB_MODE_CLEAR)
	polygon->level = (run_code * 4) - 32;
      else if(nd->nids_header.pdb_mode == NIDS_PDB_MODE_MAINTENANCE)
	log_errx(1, "Radar is in maintenance mode.");
      else
	log_errx(1, "Invalid value of radar operational mode.");

      /* XXX
      int k;
      for(k = 0; k < 4; ++k){
	fprintf(stdout, "%.2f:%.2f,", polygon->lon[k], polygon->lat[k]);
      }
      fprintf(stdout, "%d\n", polygon->level);
      */

      ++polygon;
      ++numpolygons;
    }

    /* XXX
    fprintf(stdout, "\ntotal_bins: %d\n", total_bins);
    fprintf(stdout, "\n");
    */
  }

  assert(numpolygons <= nd->polygon_map.numpolygons);
  nd->polygon_map.numpolygons = numpolygons;

  dcnids_polygonmap_bb(&nd->polygon_map);

  /* XXX
  fprintf(stdout, "\nnumpoints= %d, numpolygons = %d:%d\n",
	  numpoints, numpolygons, nd->polygon_map.numpolygons);
  */
}

static void nids_csv_write(struct nids_data_st *nd){
  /*
   * Output the polygon data.
   */
  FILE *fp;
  struct dcnids_polygon_map_st *pm = &nd->polygon_map;

  if(strcmp(g.opt_csvfile, "-") == 0)
    fp = stdout;
  else
    fp = fopen(g.opt_csvfile, "w");

  if(dcnids_csv_write(fp, pm) != 0)
    log_errx(1, "Cannot write to csv file.");

  if(fp != stdout)
    fclose(fp);
}

#ifdef PRINT_TEST
static void test_print(struct nids_data_st *nd){

  fprintf(stdout, "\n%d %d %d %d %d %u %d %u\n",
	  nd->nids_header.pdb_version,
	  nd->nids_header.pdb_symbol_block_offset,
	  nd->nids_header.pdb_graphic_block_offset,
	  nd->nids_header.pdb_tabular_block_offset,
	  nd->psb.blockid,	/* should be 1 */
	  nd->psb.blocklength,
	  nd->psb.numlayers,
	  nd->psb.psb_layer_blocklength);
}
#endif

static void nids_shp_write(struct nids_data_st *nd){

  struct dcnids_polygon_map_st *pm = &nd->polygon_map;
  int shp_fd, shx_fd;

  shp_fd = open(g.opt_shpfile, O_WRONLY | O_TRUNC | O_CREAT, 0644);
  if(shp_fd == -1)
    log_err(1, "Cannot open shp file.");

  shx_fd = open(g.opt_shxfile, O_WRONLY | O_TRUNC | O_CREAT, 0644);
  if(shx_fd == -1){
    close(shp_fd);
    log_err(1, "Cannot open shx file.");
  }

  if(dcnids_shp_write(shp_fd, shx_fd, pm) != 0){
    close(shp_fd);
    close(shx_fd);
    log_err(1, "Could not write shp or shx file.");
  }

  close(shp_fd);
  close(shx_fd);
}

static void nids_dbf_write(struct nids_data_st *nd){

  struct dcnids_polygon_map_st *pm = &nd->polygon_map;
  int status;

  /*
  status = dcnids_dbf_write(g.opt_dbffile,
			    NIDS_DBF_CODENAME, NIDS_DBF_LEVELNAME, pm);
  */
  status = dcnids_dbf_write(g.opt_dbffile, pm);

  if(status != 0)
    log_errx(1, "Error writing dbf file: %d", status);
}

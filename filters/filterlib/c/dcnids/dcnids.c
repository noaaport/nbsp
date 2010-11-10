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
#include "dcnids_decode.h"

/*
 * Usage: nbspradinfo [-c <count>] [output options] <file> | < <file>
 *        nbspdcnids  [-c <count>] [output options] <file> | < <file>
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
 * or, if the files that have an uncompressed nids header
 *
 * nbspradinfo -c 54 tjsj_sdus52-n0qjua.152011_99050818
 * nbspradinfo -c 41 n0qjua_20101015_1936.nids
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
 * If [-t] is given then only the "seconds" is printed, and if [-l] is given
 * then the m_msglength is also printed.
 *
 * Otherwise anyone of
 *
 *  -v <csv file>
 *  -p <shp file>
 *  -x <shx file>
 *  -f <dbf file>
 *  -o <info file> 
 *
 * will write the corresponding data file.
 */

struct {
  int opt_background;	/* -b */
  int opt_skipcount;	/* -c <count> => skip the first <count> bytes */
  int opt_timeonly;	/* -t => only extract and print the time (unix secs) */
  int opt_lengthonly;	/* -l => only extract and print the m_msglength */
  int opt_filter;	/* -F => apply data filtering options */
  char *opt_levelmin;	/* -m => filter "level" min value */
  char *opt_levelmax;	/* -n => filter "level" max value */
  char *opt_dbffile;	/* -f => write dbf file */
  char *opt_infofile;   /* -o => write info file */
  char *opt_shpfile;	/* -p => write shp file */
  char *opt_csvfile;	/* -v => write csv file */
  char *opt_shxfile;	/* -x => write shx file */
  char *opt_inputfile;
  /* variables */
  int opt_fopvx;	/* set if anyone of fpvx is given */
  int fd;
  int level_min;	/* data filter values */
  int level_max;
} g = {0, 0, 0, 0, 0,
       NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
       0, -1,
       0, 0};

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
static void nids_info_write(struct nids_data_st *nids_data);

/* #define PRINT_TEST */
#ifdef PRINT_TEST
static void test_print(struct nids_data_st *nids_data);
#endif

static void cleanup(void){

  if((g.fd != -1) && (g.opt_inputfile != NULL))
    (void)close(g.fd);
}

int main(int argc, char **argv){

  char *optstr = "bFltc:f:m:n:o:p:v:x:";
  char *usage = "nbspdcnids [-b] [-F] [-l] [-t] [-c count] [-f dbffile] "
    "[-m min] [-n max] [-o infofile] [-p shpfile] [-v csvfile] "
    "[-x shxfile] <file> | < file";
  int status = 0;
  int c;
  int opt_px = 0;	/* -p and -x must be given together */

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'F':
      g.opt_filter = 1;
      break;
    case 'c':
      status = strto_int(optarg, &g.opt_skipcount);
      if((status == 1) || (g.opt_skipcount <= 0)){
	log_errx(1, "Invalid argument to [-c] option.");
      }
      break;
    case 'f':
      g.opt_dbffile = optarg;
      g.opt_fopvx = 1;
      break;
    case 'l':
      g.opt_lengthonly = 1;
      break;
    case 'm':
      g.opt_levelmin = optarg;
      if(sscanf(optarg, "%d", &g.level_min) != 1){
	log_errx(1, "Invalid argument to -m option: %s", optarg);
      }
    case 'n':
      g.opt_levelmax = optarg;
      if(sscanf(optarg, "%d", &g.level_max) != 1){
	log_errx(1, "Invalid argument to -n option: %s", optarg);
      }
      break;
    case 'o':
      g.opt_infofile = optarg;
      g.opt_fopvx = 1;
      break;
    case 'p':
      g.opt_shpfile = optarg;
      g.opt_fopvx = 1;
      ++opt_px;
      break;
    case 't':
      g.opt_timeonly = 1;
      break;
    case 'v':
      g.opt_csvfile = optarg;
      g.opt_fopvx = 1;
      break;
    case 'x':
      g.opt_shxfile = optarg;
      g.opt_fopvx = 1;
      ++opt_px;
      break;
    default:
      log_info(usage);
      exit(0);
      break;
    }
  }

  if((g.opt_fopvx != 0) && ((g.opt_timeonly != 0) || (g.opt_lengthonly != 0)))
    log_errx(1, "Invalid combination of options: lt and fopvx");

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

  if((g.opt_timeonly != 0) && (g.opt_lengthonly != 0))
    fprintf(stdout, "%u %u",
	    nids_data.nids_header.unixseconds,
	    nids_data.nids_header.m_msglength);
  else if(g.opt_timeonly != 0)
    fprintf(stdout, "%u", nids_data.nids_header.unixseconds);
  else if(g.opt_lengthonly != 0)
    fprintf(stdout, "%u", nids_data.nids_header.m_msglength);
  else if(g.opt_fopvx == 0){
    fprintf(stdout, "%.3f %.3f %d %u %d %d",
	    nids_data.nids_header.lat,
	    nids_data.nids_header.lon,
	    nids_data.nids_header.pdb_height,
	    nids_data.nids_header.unixseconds,
	    nids_data.nids_header.pdb_mode,
	    nids_data.nids_header.pdb_code);
  }

  if(g.opt_fopvx == 0){
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
   * Decode the polygon data.
   */

  n = nids_data.nids_header.m_msglength - NIDS_HEADER_SIZE;
  if(n <= 0)
    log_errx(1, "Corrupt file.");

  nids_data.data = malloc(n);
  if(nids_data.data == NULL)
    log_err(1, "Error from malloc()");

  nids_data.data_size = n;
  n = read(fd, nids_data.data, nids_data.data_size);
  if(n == -1)
    log_err(1, "Error from read()");     
  else if((unsigned int)n != nids_data.data_size){
    log_errx(1, "Corrupt file.");
  }

  nids_decode_data(&nids_data);

  /* Output data */
  if(g.opt_csvfile != NULL)
    nids_csv_write(&nids_data);

  if((g.opt_shpfile != NULL) && (g.opt_shxfile != NULL))
    nids_shp_write(&nids_data);

  if(g.opt_dbffile != NULL)
    nids_dbf_write(&nids_data);

  if(g.opt_infofile != NULL)
    nids_info_write(&nids_data);

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
  int packet_code;
  int divider;
  int status, bzerror;

  /*
   * The "divider" should be -1 for legacy products, but for the new
   * products that are bz2 compressed the first two bytes are "BZ".
   * In the first case we continue, but if the divider is not -1
   * we assume it is a new product and try to send it to libbz2.
   */
  divider = (int)extract_int16(b, 1);
  if(divider != -1){
    status = dcnids_bunz(&nd->data, &nd->data_size, &bzerror);
    if(status == 1)
      log_errx(1, "Error from libbz2: %d", bzerror);
    else if(status == -1)
      log_err(1, "Error from libbz2.");

    /* repoint b */
    b = nd->data;
  }

  divider = (int)extract_int16(b, 1);
  if(divider != -1)
    log_errx(1, "Corrupt file.");

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
  packet_code = extract_uint16(b, 1);
  
  /*
   * XXX
   * To find out the product and packet codes:
   *  fprintf(stdout, "%d %d %d\n",
   *	  nd->nids_header.m_code,
   *	  nd->nids_header.pdb_code,
   *	  packet_code);
   * exit(0);
   */

  if((packet_code != NIDS_PACKET_RADIALS_AF1F) &&
     (packet_code != NIDS_PACKET_DIGITAL_RADIALS_16))
    log_errx(1, "Unsupported packet code: %d", packet_code);

  nd->radial_packet_header.packet_code = packet_code;
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
   * Here we extract the polygon data. Only the polygons that have 
   * level values within the specified limits will be included if
   * the option to use the filter is set. The filter is used only
   * for bref.
   */
  nd->polygon_map.flag_usefilter = 0;	/* default*/
  if(g.opt_filter == 1){
    nd->polygon_map.flag_usefilter = 1;

    /*
     * The limits depend on the product code.
     */
    if((nd->nids_header.pdb_code == NIDS_PDB_CODE_NXR) ||
       (nd->nids_header.pdb_code == NIDS_PDB_CODE_N0Z) ||
       (nd->nids_header.pdb_code == NIDS_PDB_CODE_NXQ)){
      nd->polygon_map.level_min = NIDS_BREF_LEVEL_MIN_VAL;
      nd->polygon_map.level_max = NIDS_BREF_LEVEL_MAX_VAL;
    }else if((nd->nids_header.pdb_code == NIDS_PDB_CODE_NXV) ||
	     (nd->nids_header.pdb_code == NIDS_PDB_CODE_NXU)){
      nd->polygon_map.level_min = NIDS_RVEL_LEVEL_MIN_VAL;
      nd->polygon_map.level_max = NIDS_RVEL_LEVEL_MAX_VAL;
    }else
      log_errx(1, "Invalid value of nd->nids_header.pdb_code.");

    /*
     * If the user specified min and max the use them.
     */
    if(g.level_min != g.level_max){
      nd->polygon_map.level_min = g.level_min;
      nd->polygon_map.level_max = g.level_max;
    }
  }

  /* 
   * The layout of the "run bins" in the radials depends on the packet type.
   */
  if(packet_code == NIDS_PACKET_RADIALS_AF1F)
    nids_decode_radials_af1f(nd);
  else if(packet_code == NIDS_PACKET_DIGITAL_RADIALS_16)
    nids_decode_digital_radials_16(nd);
  else{
    /* Already caught above */
    log_errx(1, "Unsupported packet code: %d", packet_code);
  }
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

static void nids_info_write(struct nids_data_st *nd){

  FILE *f;
  int n;

  f = fopen(g.opt_infofile, "w");
  if(f == NULL)
    log_err(1, "Error writing info file.");

  n = fprintf(f, "radseconds: %u\n", nd->nids_header.unixseconds);
  if(n > 0)
    n = fprintf(f, "radmode: %d\n", nd->nids_header.pdb_mode);

  if(n > 0)
    n = fprintf(f, "prodcode: %d\n", nd->nids_header.pdb_code);

  if(n > 0)
    n = fprintf(f, "packetcode: %d\n", nd->radial_packet_header.packet_code);

  if(n > 0)
    n = fprintf(f, "lon: %.3f\n", nd->nids_header.lon);

  if(n > 0)
    n = fprintf(f, "lat: %.3f\n", nd->nids_header.lat);

  if(n > 0)
    n = fprintf(f, "height: %d\n", nd->nids_header.pdb_height);

  fclose(f);

  if(n < 0)
    log_err(1, "Error writing info file.");
}

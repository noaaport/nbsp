/*
 * Copyright (c) 2005-2014 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id: nbspradgis.c,v 720f4f945739 2015/01/18 04:14:56 nieves $
 */

/*
 * nbspradgis [-c <count> | -C] [output options] <file> | < <file>
 *
 * The program reads from a file or stdin, but the data must start with the
 * wmo header (i.e., the ccb must have been removed). The [-c] amd [-C]
 * options work as in nbspradinfo (See nbspradinfo.c.)
 *
 * The output options are:
 *
 *  -a => same as FOPVX (all) with the default names
 *  -F => do dbf
 *  -O => do info
 *  -P => do shp
 *  -V => do csv
 *  -X => do shx
 *  -f <dbf file>
 *  -n <base name> => default base name for files
 *  -o <info file>
 *  -p <shp file>
 *  -v <csv file>
 *  -x <shx file>
 *
 * The default action is the same as specifying "-FOPX" (excluding csv).
 *
 * The data is output in the following units
 *
 *  reflectivity: dbZ
 *  rad and rel velocity: knots
 *  acumulated rain: hundredth of inch (nids_decode_nxp_codetolevel())
 */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>
#include <math.h>
#include "const.h"
#include "err.h"
#include "io.h"
#include "util.h"
#include "misc.h"
#include "dcnids.h"
#include "dcnids_extract.h"
#include "dcnids_decode.h"
#include "dcnids_misc.h"
#include "dcnids_name.h"
#include "dcnids_header.h"
#include "dcnids_info.h"

struct {
  char *opt_inputfile;
  char *opt_output_dir;	/* -d */
  char *opt_basename;   /* -n => default basename */
  int opt_all;          /* -a */
  int opt_background;	/* -b */
  int opt_skipcount;	/* -c <count> => skip the first <count> bytes */
  int opt_filter;	/* -D => apply data filtering options */
  char *opt_levelmin;	/* -M => filter "level" min value */
  char *opt_levelmax;	/* -N => filter "level" max value */
  int opt_dbf;		/* -F */
  int opt_info;		/* -O */
  int opt_shp;		/* -P */
  int opt_shx;		/* -X */
  int opt_csv;		/* -V */
  char *opt_dbffile;	/* -f => write dbf file */
  char *opt_infofile;   /* -o => write info file */
  char *opt_shpfile;	/* -p => write shp file */
  char *opt_csvfile;	/* -v => write csv file */
  char *opt_shxfile;	/* -x => write shx file */
  /* variables */
  int opt_aFOPVX;	/* set if anyone of FOPVX is given */
  int fd;
  int level_min;	/* data filter values */
  int level_max;
} g = {NULL, NULL, NULL,
       0, 0, 0, 0,
       NULL, NULL,
       0, 0, 0, 0, 0,
       NULL, NULL, NULL, NULL, NULL,
       0, -1, 0, 0};

/* general functions */
static int process_file(void);
static void cleanup(void);

/* decoding functions */
static void nids_decode_data(struct nids_data_st *nids_data);

/* output functions */
static char *nids_file_name(struct nids_header_st *nheader,
			    char *opt_file,
			    char *suffix);
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

  char *optstr = "abCDFOPVXc:d:M:N:f:n:o:p:v:x:";
  char *usage = "nbspradgis [-a] [-b] [-C] [-D] [-FOPVX] "
    "[-c count] [-d outputdir] [-M min_level] [-N min_level] "
    "[-f dbffile] [-o infofile] [-p shpfile] [-v csvfile] [-x shxfile] "
    "<file> | < file";
  int status = 0;
  int c;
  int opt_cC = 0;	/* c and C together is a conflict */

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'a':
      ++g.opt_aFOPVX;
      g.opt_dbf = 1;
      g.opt_info = 1;
      g.opt_shp = 1;
      g.opt_csv = 1;
      g.opt_shx = 1;
      break;
    case 'b':
      g.opt_background = 1;
      break;
    case 'C':
      ++opt_cC;
      g.opt_skipcount = CCB_SIZE;
      break;
    case 'D':
      g.opt_filter = 1;
      break;
    case 'F':
      ++g.opt_aFOPVX;
      g.opt_dbf = 1;
      break;
    case 'O':
      ++g.opt_aFOPVX;
      g.opt_info = 1;
      break;
    case 'P':
      ++g.opt_aFOPVX;
      g.opt_shp = 1;
      break;
    case 'V':
      ++g.opt_aFOPVX;
      g.opt_csv = 1;
      break;
    case 'X':
      ++g.opt_aFOPVX;
      g.opt_shx = 1;
      break;
    case 'c':
      ++opt_cC;
      status = strto_int(optarg, &g.opt_skipcount);
      if((status == 1) || (g.opt_skipcount <= 0)){
	log_errx(1, "Invalid argument to [-c] option.");
      }
      break;
    case 'd':
      g.opt_output_dir = optarg;
      break;
    case 'n':
      g.opt_basename = optarg;
      break;
    case 'M':
      g.opt_levelmin = optarg;
      if(sscanf(optarg, "%d", &g.level_min) != 1){
	log_errx(1, "Invalid argument to -M option: %s", optarg);
      }
      break;
    case 'N':
      g.opt_levelmax = optarg;
      if(sscanf(optarg, "%d", &g.level_max) != 1){
	log_errx(1, "Invalid argument to -N option: %s", optarg);
      }
      break;
    case 'f':
      g.opt_dbf = 1;
      ++g.opt_aFOPVX;
      g.opt_dbffile = optarg;
      break;
    case 'o':
      g.opt_info = 1;
      ++g.opt_aFOPVX;
      g.opt_infofile = optarg;
      break;
    case 'p':
      g.opt_shp = 1;
      ++g.opt_aFOPVX;
      g.opt_shpfile = optarg;
      break;
    case 'v':
      g.opt_csv = 1;
      ++g.opt_aFOPVX;
      g.opt_csvfile = optarg;
      break;
    case 'x':
      g.opt_shx = 1;
      ++g.opt_aFOPVX;
      g.opt_shxfile = optarg;
      break;
    default:
      log_info(usage);
      exit(0);
      break;
    }
  }

  /* The default is to do everything except csv */
  if(g.opt_aFOPVX == 0){
      g.opt_dbf = 1;
      g.opt_info = 1;
      g.opt_shp = 1;
      /* g.opt_csv = 1; */
      g.opt_shx = 1;
  }

  if(g.opt_background == 1)
    set_usesyslog();

  if(opt_cC >= 2)
    log_errx(1, "Invalid combination of options: c and C.");

  if(optind < argc - 1)
    log_errx(1, "Too many arguments.");
  else if(optind == argc - 1)
    g.opt_inputfile = argv[optind++];

  /*
   * The -D option is on by default since we use the "grided" version
   * of the nids->shapefile decode functions. This option is needed for the
   * legacy types (n0r, ...) otherwise the unnecesary processing of data
   * slows down the conversion and makes the files much larger.
   * (nids_decode_radials_af1f_grided())
   */
  g.opt_filter = 1;

  atexit(cleanup);
  status = process_file();

  return(status != 0 ? 1 : 0);
}

int process_file(void){

  int fd;
  int n;
  struct nids_data_st nids_data;
  int status;

  if(g.opt_inputfile == NULL)
    fd = fileno(stdin);
  else{
    fd = open(g.opt_inputfile, O_RDONLY);
    if(fd == -1)
      log_err_open(g.opt_inputfile);
    else
      g.fd = fd;
  }

  status = dcnids_read_header(fd, g.opt_skipcount, &nids_data.nids_header);
  if(status != 0){
    if(g.opt_inputfile != NULL)
      log_warnx("Error reading nids header from %s", g.opt_inputfile);
  
    if(status == -1)
      log_err(1, "Error reading file.");
    else
      log_errx(1, "Error reading file. Short file.");
  }

  if(dcnids_verify_wmoawips_header(nids_data.nids_header.buffer) != 0) {
    if(g.opt_inputfile != NULL)
      log_errx(1, "Invalid wmo header:  %s", g.opt_inputfile);
    else 
      log_errx(1, "Invalid wmo header.");
  }

  if(dcnids_decode_header(&nids_data.nids_header) != 0) {
    if(g.opt_inputfile != NULL)
      log_errx(1, "Invalid pdb header; maybe a zlib compressed header: %s",
	       g.opt_inputfile);
    else 
      log_errx(1, "Invalid pdb header; maybe a zlib compressed header.");
  }
  
  /*
   * Decode the polygon data.
   */

  n = nids_data.nids_header.m_msglength - NIDS_HEADER_SIZE;
  if(n <= 0)
    log_errx(1, "Corrupt file header.");

  nids_data.data = malloc(n);
  if(nids_data.data == NULL)
    log_err(1, "Cannot load data in memory");

  nids_data.data_size = n;
  n = readf(fd, nids_data.data, nids_data.data_size);
  if(n == -1)
    log_err(1, "Error from read()");     
  else if((unsigned int)n != nids_data.data_size){
    log_errx(1, "File is corrupt (short)");
  }

  (void)close(fd);
  g.fd = -1;

  nids_decode_data(&nids_data);

  /* Output data */

  if(g.opt_output_dir != NULL){
    if(chdir(g.opt_output_dir) != 0)
      log_err(1, "Cannot chdir to %s", g.opt_output_dir);
  }

  if(g.opt_csv != 0)
    nids_csv_write(&nids_data);

  if((g.opt_shp != 0) || (g.opt_shx != 0))
    nids_shp_write(&nids_data);

  if(g.opt_dbf != 0)
    nids_dbf_write(&nids_data);

  if(g.opt_info != 0)
    nids_info_write(&nids_data);

  return(0);
}

/*
 * decoding functions
 */
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
    } else if((nd->nids_header.pdb_code == NIDS_PDB_CODE_NXV) ||
	     (nd->nids_header.pdb_code == NIDS_PDB_CODE_NXU)){
      nd->polygon_map.level_min = NIDS_RVEL_LEVEL_MIN_VAL;
      nd->polygon_map.level_max = NIDS_RVEL_LEVEL_MAX_VAL;
    } else if((nd->nids_header.pdb_code == NIDS_PDB_CODE_NXP) ||
	      (nd->nids_header.pdb_code == NIDS_PDB_CODE_NTP)){
      nd->polygon_map.level_min = NIDS_NXP_LEVEL_MIN_VAL;
      nd->polygon_map.level_max = NIDS_NXP_LEVEL_MAX_VAL;
    } else if(nd->nids_header.pdb_code == NIDS_PDB_CODE_NXS){
      nd->polygon_map.level_min = NIDS_SRVEL_LEVEL_MIN_VAL;
      nd->polygon_map.level_max = NIDS_SRVEL_LEVEL_MAX_VAL;
    } else
	log_errx(1, "Unsupported value [%d] of nd->nids_header.pdb_code.",
		 nd->nids_header.pdb_code);
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
  else {
    /* Already caught above */
    log_errx(1, "Unsupported packet code: %d", packet_code);
  }
}

static char *nids_file_name(struct nids_header_st *nheader,
			   char *opt_basename,
			   char *suffix){
  char *file;

  if(opt_basename != NULL)
    file = dcnids_optional_name(opt_basename, suffix);
  else
    file = dcnids_default_name(nheader, NULL, suffix);
  
  if(file == NULL)
    log_err(1, "Cannot create nids_file_name()");

  return(file);
}

static void nids_csv_write(struct nids_data_st *nd){
  /*
   * Output the polygon data.
   */
  char *csvfile;
  FILE *fp;
  struct dcnids_polygon_map_st *pm = &nd->polygon_map;

  if(g.opt_csvfile != NULL)
    csvfile = g.opt_csvfile;
  else
    csvfile = nids_file_name(&nd->nids_header, g.opt_basename, DCNIDS_CSVEXT);

  if(csvfile == NULL)
    return;

  if(strcmp(csvfile, "-") == 0)
    fp = stdout;
  else
    fp = fopen(csvfile, "w");

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

  int status = 0;
  char *shpfile = NULL;
  char *shxfile = NULL;

  if(g.opt_shpfile != NULL)
    shpfile = g.opt_shpfile;
  else if(g.opt_shp != 0){
    shpfile = nids_file_name(&nd->nids_header, g.opt_basename, DCNIDS_SHPEXT);
    if(shpfile == NULL)
      return;
  }

  if(g.opt_shxfile != NULL)
    shxfile = g.opt_shxfile;
  else if(g.opt_shx != 0){
    shxfile = nids_file_name(&nd->nids_header, g.opt_basename, DCNIDS_SHXEXT);
    if(shxfile == NULL)
      return;
  }

  status = dcnids_shp_write(shpfile, shxfile, &nd->polygon_map);

  if(status != 0)
    log_err(1, "Could not write shp data.");
}

static void nids_dbf_write(struct nids_data_st *nd){

  char *dbffile;
  struct dcnids_polygon_map_st *pm = &nd->polygon_map;
  int status;

  if(g.opt_dbffile != NULL)
    dbffile = g.opt_dbffile;
  else
    dbffile = nids_file_name(&nd->nids_header, g.opt_basename, DCNIDS_DBFEXT);

  if(dbffile == NULL)
    return;

  /*
  status = dcnids_dbf_write(dbffile,
			    NIDS_DBF_CODENAME, NIDS_DBF_LEVELNAME, pm);
  */
  status = dcnids_dbf_write(dbffile, pm);

  if(status != 0)
    log_errx(1, "Error writing dbf file: %d", status);
}

static void nids_info_write(struct nids_data_st *nd){

  char *infofile;

  if(g.opt_infofile != NULL)
    infofile = g.opt_infofile;
  else
    infofile = nids_file_name(&nd->nids_header, g.opt_basename,
			      DCNIDS_INFOEXT);
  if(infofile == NULL)
    return;

  if(dcnids_info_write(infofile, nd) != 0)
    log_err(1, "Error writing info file.");
}

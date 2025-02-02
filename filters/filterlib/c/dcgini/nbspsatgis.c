/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

/*
 * Usage: nbspsatgis [output options] <file> | < <file>
 *
 * The program reads from a file or stdin, but the data must 
 * be the uncompressed gini file (including the nesdis wmo header).
 *
 * The output options are:
 *
 *  -A => do asc
 *  -F => do dbf
 *  -O => do info
 *  -P => do shp
 *  -V => do csv
 *  -X => do shx
 *  -a <asc file>
 *  -d <output dir>
 *  -f <dbf file>
 *  -n <base name> => default base name for files
 *  -o <info file>
 *  -p <shp file>
 *  -v <csv file>
 *  -x <shx file>
 *
 * The default action is the same as specifying "-FOPX" (excluding csv, asc).
 *
 * When -A is specified (asc format) the [-r] can be used to specify the
 * coordinates of the bounding box to use. The default is the "maximum
 * enclosing rectangle" The argument to the "-r" option is a string of
 * the form "lon1,lat1,lon2,lat2". For example,
 *
 *	nbspunz tige02.gini | nbspsatgis -A -r "-75,16,-64,24"
 *
 * specifies the rectangle
 *
 *	lon1,lat1 = (-75,16)
 *	lon2,lat2 = (-64,24)
 *
 * If, in addition, the [-q] option is specified, then the values 
 * are interpreted asthe amount by which to shrink the default
 * rectangle. For example, to shrink the left hand side of a tige file
 * by 5 degrees
 *
 *	nbspunz tige02.gini | nbspsatgis -A -q -r "5,0,0,0"
 *
 * Negative values will have the net effect of expanding the rectangle.
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <zlib.h>
#include <libgen.h>
#include "err.h"
#include "io.h"
#include "dcgini_util.h"
#include "dcgini_name.h"
#include "dcgini_transform.h"
#include "dcgini_shp.h"
#include "dcgini.h"

struct {
  int opt_background;		/* -b */
  int opt_llur_str_diff;	/* -q */
  int opt_asc;			/* -A */
  int opt_dbf;			/* -F */
  int opt_info;			/* -O */
  int opt_shp;			/* -P */
  int opt_shx;			/* -X */
  int opt_csv;			/* -V */
  char *opt_inputfile;
  char *opt_output_dir;		/* -d */
  char *opt_basename;           /* -n */
  char *opt_ascfile;		/* -a */
  char *opt_dbffile;		/* -f */
  char *opt_infofile;		/* -o */
  char *opt_shpfile;		/* -p */
  char *opt_llur_str;		/* -r */
  char *opt_csvfile;		/* -v */
  char *opt_shxfile;		/* -x */
  /* variables */
  int fd;
} g = {0, 0,
       0, 0, 0, 0, 0, 0,
       NULL, NULL, NULL, NULL,
       NULL, NULL, NULL, NULL, NULL, NULL,
       -1};

static int process_file(void);
static void cleanup(void);

/* output functions */
static void gini_shp_write(struct dcgini_st *dcg);
static void gini_dbf_write(struct dcgini_st *dcg);
static void gini_info_write(struct dcgini_st *dcg);
static void gini_csv_write(struct dcgini_st *dcg);
static void gini_asc_write(struct dcgini_st *dcg);

static void cleanup(void){

  if((g.fd != -1) && (g.opt_inputfile != NULL))
    (void)close(g.fd);
}

int main(int argc, char **argv){

  char *optstr = "bqAFOPVXa:d:f:n:o:p:r:v:x:";
  char *usage = "nbspsatgis [-b] [-q] [-AFOPVX] [-d outputdir]"
    " [-a <ascfile>] [-f <dbfname>] [-n <basename>] [-o <infofile>] "
    " [-p <shpname>] [-r <llurstr>] [-v <csvname>] [-x <shxmname>] [<file>]";
  int status = 0;
  int c;
  int opt_AFOPVX = 0;  /* set if any file output option is specified */

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'q':
      g.opt_llur_str_diff = 1;
      break;
    case 'A':
      ++opt_AFOPVX;
      g.opt_asc = 1;
      break;
    case 'F':
      ++opt_AFOPVX;
      g.opt_dbf = 1;
      break;
    case 'O':
      ++opt_AFOPVX;
      g.opt_info = 1;
      break;
    case 'P':
      ++opt_AFOPVX;
      g.opt_shp = 1;
      break;
    case 'V':
      ++opt_AFOPVX;
      g.opt_csv = 1;
      break;
    case 'X':
      ++opt_AFOPVX;
      g.opt_shx = 1;
      break;
    case 'd':
      g.opt_output_dir = optarg;
      break;
    case 'n':
      g.opt_basename = optarg;
      break;
    case 'a':
      g.opt_asc = 1;
      ++opt_AFOPVX;
      g.opt_ascfile = optarg;
      break;
    case 'f':
      g.opt_dbf = 1;
      ++opt_AFOPVX;
      g.opt_dbffile = optarg;
      break;
    case 'o':
      g.opt_info = 1;
      ++opt_AFOPVX;
      g.opt_infofile = optarg;
      break;
    case 'p':
      g.opt_shp = 1;
      ++opt_AFOPVX;
      g.opt_shpfile = optarg;
      break;
    case 'r':
      g.opt_llur_str = optarg;	/* for the enclosing rectangle in asc */
      break;
    case 'v':
      g.opt_csv = 1;
      ++opt_AFOPVX;
      g.opt_csvfile = optarg;
      break;
    case 'x':
      g.opt_shx = 1;
      ++opt_AFOPVX;
      g.opt_shxfile = optarg;
      break;
    default:
      log_info(usage);
      exit(0);
      break;
    }
  }

  /* The default is to do everything except csv and asc */
  if(opt_AFOPVX == 0){
      g.opt_dbf = 1;
      g.opt_info = 1;
      g.opt_shp = 1;
      g.opt_shx = 1;
      /* g.opt_csv = 1; */
      /* g.opt_asc = 1; */
  }

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

static int process_file(void){

  struct dcgini_st dcg;
  int status = 0;
  int fd;
  int n;

  /* Initialize */
  dcg.pdb.buffer_size = NESDIS_WMO_HEADER_SIZE + NESDIS_PDB_SIZE;

  if(g.opt_inputfile == NULL)
    fd = fileno(stdin);
  else{
    fd = open(g.opt_inputfile, O_RDONLY);
    if(fd == -1)
      log_err_open(g.opt_inputfile);
    else
      g.fd = fd;
  }

  status = read_nesdis_pdb(fd, &dcg.pdb);

  if(status == -1)
    log_err(1, "Error from read_nesdis_pdb()");
  else if(status == 1)
    log_errx(1, "Error from read_nesdis_pdb(). File short.");
  else if(status == 2)
    log_errx(1, "Error from read_nesdis_pdb(). File has invalid wmo header");
  
  /*
   * Read the data once and for all. Use readf() in case the input comes
   * from a pipe.
   */
  dcg.ginidata.data_size = dcg.pdb.linesize * dcg.pdb.numlines;
  dcg.ginidata.data = malloc(dcg.ginidata.data_size);
  if(dcg.ginidata.data == NULL)
    log_err(1, "Cannot load data in memory.");

  n = readf(fd, dcg.ginidata.data, dcg.ginidata.data_size);
  if(n == -1)
    log_err(1, "Error reading from file");
  else if((size_t)n != dcg.ginidata.data_size)
    log_errx(1, "Error reading from file. File is corrupt (short)");
    
  (void)close(fd);
  g.fd = -1;
      
  /*
   * Fill out point data - This function must be called unconditionally,
   * even if other data formats besised shp, csv, are needed, in order
   * to initalize properly the projection transformations and the
   * bounding box.
   */
  status = dcgini_transform_data(&dcg);

  if((status == 0) && (g.opt_asc != 0))
    status = dcgini_regrid_data_asc(&dcg, g.opt_llur_str, g.opt_llur_str_diff);

  if(status != 0)
    return(status);

  /* Output the data */

  if(g.opt_output_dir != NULL){
    status = chdir(g.opt_output_dir);
    if(status != 0)
      log_err(1, "Cannot chdir to %s", g.opt_output_dir);
  }

  if((g.opt_shp != 0) || (g.opt_shx != 0))
    gini_shp_write(&dcg);

  if(g.opt_dbf != 0)
    gini_dbf_write(&dcg);

  if(g.opt_info != 0)
    gini_info_write(&dcg);

  if(g.opt_csv != 0)
    gini_csv_write(&dcg);

  if(g.opt_asc != 0)
    gini_asc_write(&dcg);
  
  return(0);
}

/* output functions */
static void gini_shp_write(struct dcgini_st *dcg){

  char *shpfile = NULL;
  char *shxfile = NULL;

  if(g.opt_shpfile != NULL)
    shpfile = g.opt_shpfile;
  else if(g.opt_shp != 0){
    if(g.opt_basename != NULL)
      shpfile = dcgini_optional_name(g.opt_basename, DCGINI_SHPEXT);
    else
      shpfile = dcgini_default_name(&dcg->pdb, NULL, DCGINI_SHPEXT);

    if(shpfile == NULL)
      log_err(1, "dcgini_xxx_name()");
  }
    
  if(g.opt_shxfile != NULL)
    shxfile = g.opt_shxfile;
  else if(g.opt_shx != 0){
    if(g.opt_basename != NULL)
      shxfile = dcgini_optional_name(g.opt_basename, DCGINI_SHXEXT);
    else
      shxfile = dcgini_default_name(&dcg->pdb, NULL, DCGINI_SHXEXT);

    if(shxfile == NULL)
      log_err(1, "dcgini_xxx_name()");
  }

  if(dcgini_shp_write(shpfile, shxfile, &dcg->pointmap) != 0)
      log_errx(1, "Error in dcgini_shp_write_data()");
}

static void gini_dbf_write(struct dcgini_st *dcg){

  char *dbffile;

  if(g.opt_dbffile != NULL)
    dbffile = g.opt_dbffile;
  else{
    if(g.opt_basename != NULL)
      dbffile = dcgini_optional_name(g.opt_basename, DCGINI_DBFEXT);
    else
      dbffile = dcgini_default_name(&dcg->pdb, NULL, DCGINI_DBFEXT);

    if(dbffile == NULL)
      log_err(1, "dcgini_default_name()");
  }

  if(dcgini_dbf_write(dbffile, &dcg->pointmap) != 0)
    log_errx(1, "Error in dcgini_dbf_write_data()");
}

static void gini_info_write(struct dcgini_st *dcg){

  char *infofile;

  if(g.opt_infofile != NULL)
    infofile = g.opt_infofile;
  else{
    if(g.opt_basename != NULL)
      infofile = dcgini_optional_name(g.opt_basename, DCGINI_INFOEXT);
    else
      infofile = dcgini_default_name(&dcg->pdb, NULL, DCGINI_INFOEXT);

    if(infofile == NULL)
      log_err(1, "dcgini_default_name()");
  }

  if(dcgini_info_write(infofile, dcg) != 0)
    log_errx(1, "Error in dcgini_info_write_data()");
}

static void gini_csv_write(struct dcgini_st *dcg){

  char *csvfile;

  if(g.opt_csvfile != NULL)
    csvfile = g.opt_csvfile;
  else{
    if(g.opt_basename != NULL)
      csvfile = dcgini_optional_name(g.opt_basename, DCGINI_CSVEXT);
    else
      csvfile = dcgini_default_name(&dcg->pdb, NULL, DCGINI_CSVEXT);

    if(csvfile == NULL)
      log_err(1, "Error from dcgini_default_name()");
  }

  if(dcgini_csv_write(csvfile, &dcg->pointmap) != 0)
    log_errx(1, "Error in dcgini_csv_write_data()");
}

static void gini_asc_write(struct dcgini_st *dcg){

  char *ascfile;

  if(g.opt_ascfile != NULL)
    ascfile = g.opt_ascfile;
  else{
    if(g.opt_basename != NULL)
      ascfile = dcgini_optional_name(g.opt_basename, DCGINI_ASCEXT);
    else
      ascfile = dcgini_default_name(&dcg->pdb, NULL, DCGINI_ASCEXT);

    if(ascfile == NULL)
      log_err(1, "Error from dcgini_default_name()");
  }

  if(dcgini_asc_write(ascfile, &dcg->gridmap) != 0)
    log_errx(1, "Error in dcgini_asc_write_data()");
}

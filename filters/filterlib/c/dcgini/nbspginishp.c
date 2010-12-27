/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
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
#include <string.h>
#include <png.h>
#include <zlib.h>
#include <libgen.h>
#include "err.h"
#include "io.h"
#include "dcgini_util.h"
#include "dcgini_name.h"
#include "dcgini_transform.h"
#include "dcgini_shp.h"
#include "dcgini.h"

/*
 * Usage: nbspginishp [output options] <file> | < <file>
 *
 * The program reads from a file or stdin, but the data must 
 * be the uncompressed gini file. The output options are:
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
 */

struct {
  char *opt_inputfile;
  char *opt_output_dir;		/* -d */
  char *opt_basename;           /* -n */
  int opt_all;			/* -a */
  int opt_background;		/* -b */
  int opt_silent;		/* -s */
  char *opt_dbffile;		/* -f */
  char *opt_infofile;		/* -o */
  char *opt_shpfile;		/* -p */
  char *opt_shxfile;		/* -x */
  char *opt_csvfile;		/* -v */
  int opt_dbf;			/* -F */
  int opt_info;			/* -O */
  int opt_shp;			/* -P */
  int opt_shx;			/* -X */
  int opt_csv;			/* -V */
  /* variables */
  int fd;
} g = {NULL, NULL, NULL,
       0, 0, 0,
       NULL, NULL, NULL, NULL, NULL,
       0, 0, 0, 0, 0,
       -1};

static int process_file(void);
static void cleanup(void);

/* output functions */
static void gini_shp_write(struct dcgini_st *dcg);
static void gini_dbf_write(struct dcgini_st *dcg);
static void gini_info_write(struct dcgini_st *dcg);
static void gini_csv_write(struct dcgini_st *dcg);

static void cleanup(void){

  if((g.fd != -1) && (g.opt_inputfile != NULL))
    (void)close(g.fd);
}

int main(int argc, char **argv){

  char *optstr = "absFOPVXd:f:n:o:p:v:x:";
  char *usage = "nbspginishp [-a] [-b] [-s] [-FOPVX] [-d outputdir]"
    " [-f <dbfname>] [-n <basename>] [-o <infofile>] [-p <shpname>]"
    " [-v <csvname>] [-x <shxmname>] [<file>]";
  int status = 0;
  int c;
  int opt_aFOPVX = 0;  /* set if any file output option is specified */

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'a':
      ++opt_aFOPVX;
      g.opt_dbf = 1;
      g.opt_info = 1;
      g.opt_shp = 1;
      g.opt_csv = 1;
      g.opt_shx = 1;
      break;
    case 'b':
      g.opt_background = 1;
      break;
    case 's':
      g.opt_silent = 1;
      break;
    case 'F':
      ++opt_aFOPVX;
      g.opt_dbf = 1;
      break;
    case 'O':
      ++opt_aFOPVX;
      g.opt_info = 1;
      break;
    case 'P':
      ++opt_aFOPVX;
      g.opt_shp = 1;
      break;
    case 'V':
      ++opt_aFOPVX;
      g.opt_csv = 1;
      break;
    case 'X':
      ++opt_aFOPVX;
      g.opt_shx = 1;
      break;
    case 'd':
      g.opt_output_dir = optarg;
      break;
    case 'n':
      g.opt_basename = optarg;
      break;
    case 'f':
      g.opt_dbf = 1;
      ++opt_aFOPVX;
      g.opt_dbffile = optarg;
      break;
    case 'o':
      g.opt_info = 1;
      ++opt_aFOPVX;
      g.opt_infofile = optarg;
      break;
    case 'p':
      g.opt_shp = 1;
      ++opt_aFOPVX;
      g.opt_shpfile = optarg;
      break;
    case 'v':
      g.opt_csv = 1;
      ++opt_aFOPVX;
      g.opt_csvfile = optarg;
      break;
    case 'x':
      g.opt_shx = 1;
      ++opt_aFOPVX;
      g.opt_shxfile = optarg;
      break;
    default:
      log_info(usage);
      exit(0);
      break;
    }
  }

  /* The default is to do everything except csv */
  if(opt_aFOPVX == 0){
      g.opt_dbf = 1;
      g.opt_info = 1;
      g.opt_shp = 1;
      /* g.opt_csv = 1; */
      g.opt_shx = 1;
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
      
  /* Fill out point data - dcgini_transform.c */
  status = dcgini_transform_data(&dcg);

  if(status != 0)
    return(status);

  /* Output the shp data */

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

/*
 * Copyright (c) 2025 Jose F. Nieves <nieves@ltp.uprrpedu>
 *
 * See LICENSE
 *
 * $Id$
 */

/*
 * Usage: nbspgoesrgis [output_options] [options] <file>
 *
 * The output_options are:
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
 *  -n <base name> => default base name for output files not specified
 *  -o <info file>
 *  -p <shp file>
 *  -v <csv file>
 *  -x <shx file>
 *
 * The default action is the same as specifying "-FOPX" (excluding csv, asc).
 *
 * When -A is specified (asc format) the [-s] can be used to specify the
 * coordinates of the bounding box to use. The default is the "maximum
 * enclosing rectangle" The argument to the "-r" option is a string of
 * the form "lon1,lat1,lon2,lat2". For example,
 *
 *   nbspgoesrgis -A -s "-75,16,-64,24" tire05.nc
 *
 * specifies the rectangle
 *
 *	lon1,lat1 = (-75,16)
 *	lon2,lat2 = (-64,24)
 *
 * If, in addition, the [-t] option is specified, then the values 
 * are interpreted as the amount by which to shrink the default
 * rectangle. For example, to shrink the left hand side of a tire file
 * by 5 degrees
 *
 *    nbspgoesgis -A -t -s "5,0,0,0" tire05.nc
 *
 * Negative values will have the net effect of expanding the rectangle.
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> /* getopt */
#include <libgen.h> /* basename */
#include <string.h>
#include <err.h>
#include <math.h>
#include <netcdf.h>
#include "err.h"
#include "dcgoesr_name.h"
#include "dcgoesr_shp.h"
#include "dcgoesr_nc.h"	/* includes dcgoesr.h */

struct {
  int opt_background;		/* -b */
  int opt_llur_str_diff;	/* -t */
  int opt_glm_file;		/* -g */
  int opt_or_file;		/* -r */
  int opt_asc;			/* -A */
  int opt_dbf;			/* -F */
  int opt_info;			/* -O */
  int opt_shp;			/* -P */
  int opt_shx;			/* -X */
  int opt_csv;			/* -V */
  char *opt_inputfile;
  char *opt_output_dir;		/* -d */
  char *opt_basename;           /* -n */
  char *opt_llur_str;		/* -s */
  char *opt_ascfile;		/* -a */
  char *opt_dbffile;		/* -f */
  char *opt_infofile;		/* -o */
  char *opt_shpfile;		/* -p */
  char *opt_csvfile;		/* -v */
  char *opt_shxfile;		/* -x */
  /* variables */
  struct goesr_st *goesr;
} g = {0, 0, 0, 0,
       0, 0, 0, 0, 0, 0,
       NULL, NULL, NULL, NULL,
       NULL, NULL, NULL, NULL, NULL, NULL,
       NULL};

/* static functions */
static void log_errx_nc(int e, char *msg, int status);

static int process_file(void);
static void init(void);
static void cleanup(void);
static void load(void);
static void output(void);

static void output_shp(void);
static void output_dbf(void);
static void output_info(void);
static void output_csv(void);
static void output_asc(void);

int main(int argc, char **argv){

  char *optstr = "bgrtAFOPVXa:d:f:n:o:p:s:v:x:";
  char *usage = "nbspsatgis [-bgrt] [-AFOPVX] [-d outputdir]"
    " [-a <ascfile>] [-f <dbfname>] [-n <basename>] [-o <infofile>] "
    " [-p <shpname>] [-s <llurstr>] [-v <csvname>] [-x <shxmname>] [<file>]";
  int status = 0;
  int c;
  int opt_AFOPVX = 0;  /* set if any file output option is specified */

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'g':
      g.opt_glm_file = 1;
      break;
    case 'r':
      g.opt_or_file = 1;
      break;
    case 't':
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
    case 's':
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
  else if(optind > argc -1)
    log_errx(1, "Requires one argument.");
  
  g.opt_inputfile = argv[optind++];

  init();
  atexit(cleanup);

  status = process_file();

  return(status != 0 ? 1 : 0);
}

/* local functions */

static void log_errx_nc(int e, char *msg, int status) {

  log_errx(e, "%s: %s", msg, nc_strerror(status));
}

static void init(void) {
  /*
   * This is not required, but it is the correct thing to do. Otherwise
   * atexit(cleanup) may not work correctly if we decide to open
   * and nc file and leave it open for sometime, with a call
   * to the nc_close in cleanup() for taking care of more complicated
   * input/output and error handling. (See the Note in dev-notes/netcdf).
   */
  nc_initialize();

  /* Let the decoder functions know the type of input file */
  if(g.opt_glm_file == 1)
    goesr_config(1);
  else if(g.opt_or_file == 1)
    goesr_config(2);
}

static void cleanup(void) {

  /*
   * free the internal netcdf memory - not absolutely required but cleaner.
   */
  nc_finalize();	
   
  if(g.goesr != NULL)
    goesr_free(g.goesr);

  g.goesr = NULL;
}

static int process_file(void){

  load();		/* load the data from the nc file */
  output();		/* do whatever with the data */

  return(0);
}

static void load(void) {

  int ncid;
  int status = 0;
  int status_close = 0;

  /*
   * NC_NOWRITE tells netCDF we want read-only access
   */
  status = nc_open(g.opt_inputfile, NC_NOWRITE, &ncid);
  if(status != 0)
    log_errx_nc(1, "Error from nc_open.", status);

  status = goesr_create(ncid, &g.goesr);
  status_close = nc_close(ncid);
  
  if(status != 0) {
    if(status == -1)
      log_err(0, "%s", "Error from goesr_create.");
    else
      log_errx_nc(0, "Error from goesr_create.", status);
  }

  if(status_close != 0)
    log_errx_nc(0, "Error from nc_close:", status_close);

  if((status != 0) || (status_close != 0))
    log_errx(1, "%s", "Aborting");
}

static void output(void) {
  /*
   * Output the data
   */
  int status = 0;

  if(g.opt_output_dir != NULL){
    status = chdir(g.opt_output_dir);
    if(status != 0)
      log_err(1, "Cannot chdir to %s", g.opt_output_dir);
  }

  if((g.opt_shp != 0) || (g.opt_shx != 0))
    output_shp();

  if(g.opt_dbf != 0)
    output_dbf();

  if(g.opt_info != 0)
    output_info();

  if(g.opt_csv != 0)
    output_csv();

  if(g.opt_asc != 0)
    output_asc();
}

static void output_shp(void) {

  int status = 0;
  char *shpfile = NULL;
  char *shxfile = NULL;
  int f_free_shp = 0;
  int f_free_shx = 0;

  if(g.opt_shpfile != NULL)
    shpfile = g.opt_shpfile;
  else if(g.opt_shp != 0) {
    if(g.opt_basename != NULL)
      shpfile = dcgoesr_name(g.opt_basename, DCGOESR_SHPEXT);
      
    if(shpfile == NULL)
      log_err(1, "shp output file could not be set or not specified");
    else
      f_free_shp = 1;
  }
  
  if(g.opt_shxfile != NULL)
    shxfile = g.opt_shxfile;
  else if(g.opt_shx != 0) {
    if(g.opt_basename != NULL)
      shxfile = dcgoesr_name(g.opt_basename, DCGOESR_SHXEXT);

    if(shxfile == NULL) {
      if(f_free_shp == 1)
	free(shpfile);

      log_err(1, "shx output file could not be set or not specified");
    } else
      f_free_shx = 1;
  }
  
  status = dcgoesr_shp_write(shpfile, shxfile, &g.goesr->pmap);

  if(f_free_shp)
    free(shpfile);

  if(f_free_shx)
    free(shxfile);

  if(status != 0)
    log_errx(1, "Error in dcgoesr_shp_write()");
}

static void output_dbf(void) {

  int status = 0;
  char *dbffile = NULL;
  int f_free_dbf = 0;
  
  if(g.opt_dbffile != NULL)
    dbffile = g.opt_dbffile;
  else{
    if(g.opt_basename != NULL)
      dbffile = dcgoesr_name(g.opt_basename, DCGOESR_DBFEXT);

    if(dbffile == NULL)
      log_err(1, "dbf output file could not be set or not specified");
    else
      f_free_dbf = 1;
  }

  status = dcgoesr_dbf_write(dbffile, &g.goesr->pmap);

  if(f_free_dbf)
    free(dbffile);

  if(status != 0)
    log_errx(1, "Error in dcgoesr_dbf_write()");
}

static void output_info(void) {
  
  int status = 0;
  char *infofile = NULL;
  int f_free_info = 0;
  
  if(g.opt_infofile != NULL)
    infofile = g.opt_infofile;
  else{
    if(g.opt_basename != NULL)
      infofile = dcgoesr_name(g.opt_basename, DCGOESR_INFOEXT);

    if(infofile == NULL)
      log_err(1, "info output file could not be set or not specified");
    else
      f_free_info = 1;
  }

  status = dcgoesr_info_write(infofile, &g.goesr->pmap);

  if(f_free_info)
    free(infofile);

  if(status != 0)
    log_errx(1, "Error in dcgoesr_info_write()");
}

static void output_csv(void) {

  int status = 0;
  char *csvfile = NULL;
  int f_free_csv = 0;
  
  if(g.opt_csvfile != NULL)
    csvfile = g.opt_csvfile;
  else{
    if(g.opt_basename != NULL)
      csvfile = dcgoesr_name(g.opt_basename, DCGOESR_CSVEXT);

    if(csvfile == NULL)
      log_err(1, "csv output file could not be set or not specified");
    else
      f_free_csv = 1;
  }

  status = dcgoesr_csv_write(csvfile, &g.goesr->pmap);

  if(f_free_csv)
    free(csvfile);

  if(status != 0)
    log_errx(1, "Error in dcgoesr_csv_write()");
}

static void output_asc(void) {

  log_info("%s", "Not implemented.");
}


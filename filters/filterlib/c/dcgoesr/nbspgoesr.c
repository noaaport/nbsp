/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
/*
 * Usage: nbspgoesr [-bci] [-g|-r] [-d outputdir] [-o outputfile] <ncfile>
 *
 * -b => bakground
 * -c => output csv - default is png
 * -i => output info - default is png
 * -g|-r => input is a glm (tirs00) or an OR_ABI type file
 * -d => directory for output file
 * -o => name of output file (for png or csv) - default is stdout
 *
 * If the [-i] option is set, the following info is printed to stdout:
 *
 *   nx, ny, tile_center_lon, tile_center_lat ,lon1, lat1, lon2, lat2
 *
 * all in one line separated by a space. (lon1,lat1) and
 * (lon2,lat2) are the coordinates of the lower-left and upper-right
 * points, respectively.
 *
 * If the [-c] option is set, the output is the data in csv format,
 * (either to stdout, or the file set by [-o] if given).
 *
 * All angles are output degrees.
 *
 * If [-c] is not set, then the png is output provided [-i] is not
 * set or the [-o] is set.
 *
 * The program assumes that the input file is a noaaport file.
 * With the [-g] option the program assumes that the input is a noaaport
 * "tirs00" type file. The [-r] option indicates the input is an
 * "OR_ABI-L1b-RadF-M6C01_G16" type file.
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> /* getopt */
#include <libgen.h> /* basename */
#include <err.h>
#include <math.h>
#include <netcdf.h>
#include "err.h"
#include "dcgoesr_nc.h"
#include "dcgoesr_png.h"

struct {
  int opt_background;		/* -b */
  int opt_csv;			/* -c */
  int opt_info;			/* -i */
  int opt_glm_file;		/* -g */
  int opt_or_file;		/* -r */
  char *opt_inputfile;
  char *opt_outputfile;		/* -o */
  char *opt_outputdir;		/* -d */
  /* variables */
  struct goesr_st *goesr;
  FILE *fp;			/* output file */
} g = {0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL};

/* static functions */
static void init(void);
static void cleanup(void);
static void load(void);
static void output(void);
static int output_csv(void);
static int output_info(void);
static int output_png(void);
static void log_errx_nc(int e, char *msg, int status);

/*
 * main
 */
int main(int argc, char ** argv) {
  
  char *optstr = "hbcigrd:o:";
  char *usage = "nbspgoesr [-h] [-bcei] [-g|-r] [-d outputdir]"
    " [-o outputfile] <inputfile>";
  int c;
  int status = 0;

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'c':
      g.opt_csv = 1;
      break;
    case 'i':
      g.opt_info = 1;
      break;
    case 'g':
      g.opt_glm_file = 1;
      break;
    case 'r':
      g.opt_or_file = 1;
      break;
    case 'd':
      g.opt_outputdir = optarg;
      break;
    case 'o':
      g.opt_outputfile = optarg;
      break;
    case 'h':
    default:
      status = 1;
      log_errx(1, "%s", usage);
      break;
    }
  }

  if(g.opt_background == 1)
    set_usesyslog();

  if((g.opt_glm_file == 1) && (g.opt_or_file == 1))
    log_errx(1, "%s", "Conflicting g,r options.");

  if(optind < argc - 1)
    log_errx(1, "%s", "Too many arguments.");
  else if(optind == argc - 1)
    g.opt_inputfile = argv[optind++];
  else
    log_errx(1, "%s", "Needs inputfile as argument.");

  init();
  atexit(cleanup);
  
  load();		/* load the data from the nc file */
  output();		/* do whatever with the data */

  return(0);
}

/*
 * local functions
 */
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

  int status = 0;
  int status_close = 0;

  if(g.opt_outputdir != NULL){
    status = chdir(g.opt_outputdir);
    if(status != 0)
      log_err(1, "Cannot chdir to %s", g.opt_outputdir);
  }

  if(g.opt_outputfile != NULL) {
    g.fp = fopen(g.opt_outputfile, "w");
    if(g.fp == NULL)
      log_err(1, "Error opening %s", g.opt_outputfile);
  } else
    g.fp = stdout;

  /* If the info option is set, output the info to stdout */
  if(g.opt_info == 1)
    status = output_info();

  /*
   * If the csv option is set, output the csv. This can be to stdout
   * or whatever output file is set by the options.
   */
  if(g.opt_csv == 1)
    status = output_csv();

  /*
   * The default is png if:
   *   - neither the csv option or info are set
   *   - csv is not set and info is, but the output is set to a file.
   */
  if(g.opt_csv == 0) {
    if((g.opt_info == 0) || (g.opt_outputfile != NULL))
      status = output_png();
  }

  if((g.fp != stdout) && (g.fp != NULL)) {
    status_close = fclose(g.fp);
    g.fp = NULL;
  }

  if(status != 0)
    log_err(0, "%s %s", "Error writing to", g.opt_outputfile);

  if(status_close != 0)
    log_err(0, "%s %s", "Error closing", g.opt_outputfile);

  if((status != 0) || (status_close != 0)) {
    (void)unlink(g.opt_outputfile);
    log_errx(1, "%s", "Aborting");
  }
}  

static int output_csv(void) {

  int i, j;		/* loop indexes x[i], y[j] */
  int k;		/* "cmi(j,i)"  = cmi[k] with k = j*nx + i */
  int status = 0;
  double lon, lat, cmi;

  /* print in the order x,y, with x varying faster */
  for (j = 0; j < g.goesr->ny; ++j) {
    for (i = 0; i < g.goesr->nx; ++i) {
      k = j*g.goesr->nx + i;	/* "cmi(j,i)"  = cmi[k] with k = j*nx + i */
      
      lon = g.goesr->pmap.points[k].lon;
      lat = g.goesr->pmap.points[k].lat;
      cmi = g.goesr->cmi[k];

      if(fprintf(g.fp, "%f,%f,%f\n", lon, lat, cmi) < 0) {
	status = -1;
	goto end;
      }
    }
  }

 end:

  return(status);
}

static int output_info(void) {
  /*
   * The info is output to stdout.
   */
  int status = 0;
  double tclon, tclat, lon1, lat1, lon2, lat2;

  tclon = g.goesr->tclon;
  tclat = g.goesr->tclat;
  lon1 = g.goesr->pmap.lon1;
  lat1 = g.goesr->pmap.lat1;
  lon2 = g.goesr->pmap.lon2;
  lat2 =g.goesr->pmap.lat2;

  if(fprintf(stdout, "%d %d %f %f %f %f %f %f\n",
	     g.goesr->nx, g.goesr->ny,
	     tclon, tclat, lon1, lat1, lon2, lat2) < 0) {
    status = -1;
  }

  return(status);
}

static int output_png(void) {

  int status = 0;

  status = write_data_png(g.fp, g.goesr->level, g.goesr->nx, g.goesr->ny);

  return(status);
}

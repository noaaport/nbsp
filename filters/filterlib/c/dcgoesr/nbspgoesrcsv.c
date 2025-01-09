/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
/*
 * Usage: goesrcsv [-b] [-o outputfile] <ncfile>
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* getopt */
#include <libgen.h> /* basename */
#include <err.h>
#include <math.h>
#include <netcdf.h>
#include "err.h"
#include "dcgoesr_nc.h"

struct {
  int opt_background;
  char *opt_inputfile;
  char *opt_outputfile;
  /* variables */
  struct goesr_st *goesr;
  FILE *fp;			/* output file */
} g = {0, NULL, NULL, NULL, NULL};

/* static functions */
static void cleanup(void);
static void load(void);
static void output(void);
static int output_csv(void);
static void log_errx_nc(int e, char *msg, int status);

/*
 * main
 */
int main(int argc, char ** argv) {
  
  char *optstr = "bho:";
  char *usage = "nbspgoesrcsv [-h] [-b] [-o outputfile] <inputfile>";
  int c;
  int status = 0;

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
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

  if(optind < argc - 1)
    log_errx(1, "%s", "Too many arguments.");
  else if(optind == argc - 1)
    g.opt_inputfile = argv[optind++];
  else
    log_errx(1, "%s", "Needs inputfile as argument.");

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

static void cleanup(void) {

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
  
  if(g.opt_outputfile != NULL) {
    g.fp = fopen(g.opt_outputfile, "w");
    if(g.fp == NULL)
      log_err(1, "Error opening %s", g.opt_outputfile);
  } else
    g.fp = stdout;

  status = output_csv();
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
  
  /* print in the order x,y, with x varying faster */
  for (j = 0; j < g.goesr->ny; ++j) {
    for (i = 0; i < g.goesr->nx; ++i) {
      k = j*g.goesr->nx + i;	/* "cmi(j,i)"  = cmi[k] with k = j*nx + i */
      if(fprintf(g.fp, "%f,%f,%f\n",
		 g.goesr->lon[k], g.goesr->lat[k], g.goesr->cmi[k]) < 0) {
	status = -1;
	goto theEnd;
      }
    }
  }

 theEnd:

  return(status);
}

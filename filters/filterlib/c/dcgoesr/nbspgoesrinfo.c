/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 *
 * Usage: nbspgoesrinfo [-b] <ncfile>
 *
 * -b => bakground
 *
 * The program emits the values of the following data
 *
 * product_name
 * start_date_time
 * source_scene
 * satellite_id
 * abi_mode
 * channel_id
 *
 * all in one line, separated by a space.
 * 
 * Example - On a "tire13" file,
 *
 *       econus-020-b12-m6c13 2025011180117 conus goes-16 6 13
 *
 * The output can then be used in the filters.lib like this:
 *
 * set goesrinfo [exec nbspgoesrinfo $ncfile]
 *
 * set rc(goesr_product_name) [lindex $goesrinfo 0];
 * set rc(goesr_start_date_time) [lindex $goesrinfo 1];
 * set rc(goesr_source_scene) [lindex $goesrinfo 2];
 * set rc(goesr_satellite_id) [lindex $goesrinfo 3];
 * set rc(goesr_abi_mode) [lindex $goesrinfo 4];
 * set rc(goesr_channel_id) [lindex $goesrinfo 5];
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> /* getopt */
#include <libgen.h> /* basename */
#include <string.h> /* memset */
#include <ctype.h>  /* tolower */
#include <err.h>
#include <netcdf.h>
#include "err.h"
 
#define NPARAMS		  4	/* four text parameters */

struct goesr_global_info_st {
  char *data;		/* storage for the parameter values */
  char *key[NPARAMS];	/* keywords */
  char *param[NPARAMS];	/* pointers to data */
  int abi_mode;
  int channel_id;
} ginfo;

struct {
  int opt_background;		/* -b */
  /* variables */
  char *opt_inputfile;
} g = {0, NULL};

/* static functions */
static void log_errx_nc(int e, char *msg, int status);
static void init(void);
static void cleanup(void);
static int init_text_params(int ncid);
static int get_text_params(int ncid);
static void tolower_text(char *s);

int main(int argc, char **argv) {

  char *optstr = "bh";
  char *usage = "nbspgoesrinfo [-h] [-b] <inputfile>";
  int ncid;
  int i;
  int c;
  int status = 0;
  int status_close = 0;

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
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

  init();
  atexit(cleanup);
  
  status = nc_open(g.opt_inputfile, NC_NOWRITE, &ncid);
  if(status != 0)
    log_errx_nc(1, "Error from nc_open.", status);



  status = init_text_params(ncid);
  if(status == 0)
    status = get_text_params(ncid);
  
  if(status == 0)
    status = nc_get_att_int(ncid, NC_GLOBAL,
			     "abi_mode", &ginfo.abi_mode);

  if(status == 0)
    status = nc_get_att_int(ncid, NC_GLOBAL,
			    "channel_id", &ginfo.channel_id);
  
  status_close = nc_close(ncid);

  if(status != 0) {
    if(status == -1)
      log_err(0, "%s", "OS error.");
    else
      log_errx_nc(0, "NC error.", status);
  }

  if(status_close != 0)
    log_errx_nc(0, "Error from nc_close:", status_close);

  if((status != 0) || (status_close != 0))
    log_errx(1, "%s", "Aborting");

  for(i = 0; i < NPARAMS; ++i)
    fprintf(stdout, "%s ", ginfo.param[i]);
  
  fprintf(stdout, "%d ", ginfo.abi_mode);
  fprintf(stdout, "%d\n", ginfo.channel_id);

  return(0);
}

/*
 * local functions
 */
static void log_errx_nc(int e, char *msg, int status) {

  log_errx(e, "%s: %s", msg, nc_strerror(status));
}

static void init(void) {

  int i;

  nc_initialize();
  
  ginfo.data = NULL;
  
  ginfo.key[0] = "product_name";
  ginfo.key[1] = "start_date_time";
  ginfo.key[2] = "source_scene";
  ginfo.key[3] = "satellite_id";

  for(i = 0; i < NPARAMS; ++i)
    ginfo.param[i] = NULL;
}

static void cleanup(void) {

  int i;

  nc_finalize();

  if(ginfo.data != NULL){
    free(ginfo.data);
    ginfo.data = NULL;
  }
  
  for(i = 0; i < NPARAMS; ++i)
      ginfo.param[i] = NULL;
}

static int init_text_params(int ncid) {

  char *p;
  size_t len, maxlen;
  int i;
  int status;

  /* get the largest parameter */
  maxlen = 0;
  for(i = 0; i < NPARAMS; ++i) {
    status = nc_inq_attlen(ncid, NC_GLOBAL, ginfo.key[0] , &len);
    if(status != 0)
      break;

    if(len > maxlen)
      maxlen = len;
  }

  if(status != 0)
    return(status);

  /* space for '\0' */
  ++maxlen;

  /* allocate space for all the parameters */
  p = malloc(maxlen*NPARAMS);
  if(p == NULL)
    return(-1);

  memset(p, 0, maxlen*NPARAMS);
  
  for(i = 0; i < NPARAMS; ++i) {
      ginfo.param[i] = p;
      p += maxlen;
  }

  return(status);
}

static int get_text_params(int ncid) {

  int i;
  int status = 0;

  for(i = 0; i < NPARAMS; ++i) {
    status = nc_get_att_text(ncid, NC_GLOBAL, ginfo.key[i], ginfo.param[i]);
    if(status != 0)
      break;

    tolower_text(ginfo.param[i]);
  }

  return(status);
}

static void tolower_text(char* s) {

  char *p;

  for(p=s; *p; ++p)
    *p=tolower(*p);
}

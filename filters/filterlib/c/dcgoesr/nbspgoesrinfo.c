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
 * unix_seconds
 * ymd
 * hm
 *
 * all in one line, separated by a space. The first 6 are contained in the
 * nc file The ymd and hm are the date conversion of the julian start_date_time.
 * 
 * Example - On a "tire13" file,
 *
 *    econus-020-b12-m6c13 2025011180117 conus goes-16 6 13 \
 *                         1736982077 20250115 2301
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
 * set rc(goesr_useconds) [lindex $goesrinfo 6];
 * set rc(goesr_ymd) [lindex $goesrinfo 7];
 * set rc(goesr_hm) [lindex $goesrinfo 8];
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> /* getopt */
#include <libgen.h> /* basename */
#include <string.h> /* memset */
#include <ctype.h>  /* tolower */
#include <time.h>    /* tm - in the date-time conversion */
#include <err.h>
#include <netcdf.h>
#include "err.h"

/* Parameters obtained directly from the nc file */
#define NPARAMS		  4	/* four text parameters */

struct goesr_global_info_st {
  char *data;		/* storage for the parameter values */
  char *key[NPARAMS];	/* keywords */
  char *param[NPARAMS];	/* pointers to data */
  int abi_mode;
  int channel_id;
} ginfo;

/* for the "derived" parameters */
#define YYYYDDMM_LENTGH	  8
#define HHMM_LENTGH 4

struct goesr_derived_info_st {
  time_t seconds;	/* date converted to unix seconds */
  char ymd[YYYYDDMM_LENTGH + 1];
  char hm[HHMM_LENTGH + 1];
  int ymd_size;
  int hm_size;
} gdinfo = {0, {'\0'}, {'\0'}, YYYYDDMM_LENTGH + 1, HHMM_LENTGH + 1};

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
/* convert the julian date */
static int get_ymd_hm(char *start_date_time, time_t *seconds,
		      char *ymd, char *hm, int ymd_size, int hm_size);

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

  /* Get the derived parameters - the index of start_date_time is 1 */ 
  status = get_ymd_hm(ginfo.param[1], &gdinfo.seconds,
		      gdinfo.ymd, gdinfo.hm, 
		      gdinfo.ymd_size, gdinfo.hm_size);
  if(status != 0)
    log_errx(1, "%s", "Error converting the date and time.");

  for(i = 0; i < NPARAMS; ++i)
    fprintf(stdout, "%s ", ginfo.param[i]);
  
  fprintf(stdout, "%d ", ginfo.abi_mode);
  fprintf(stdout, "%d ", ginfo.channel_id);

  fprintf(stdout, "%lu ", gdinfo.seconds);
  fprintf(stdout, "%s ", gdinfo.ymd);
  fprintf(stdout, "%s\n", gdinfo.hm);

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

/* The derived parameters */
static int get_ymd_hm(char *start_date_time, time_t *seconds,
		      char *ymd, char *hm, int ymd_size, int hm_size) {
  /*
   * Return the yyyymmdd and hhmm portion from a string like
   * 2025045213617. It is assumed that the caller has
   * allocated storage for the results of size at least
   * 8 + 1 and 4 + 1, respectively. The "seconds" parameter
   * will contain the seconds since the epoch.
   *
   * Returns:
   * 0 => no error
   * 1 => error
   */
  struct tm tm;
  int n;	/* for the return value of sscanf */
  int year;
  int mday;
  int h, m, s;

  if(ymd_size < YYYYDDMM_LENTGH + 1)
    return(1);

  if(hm_size < HHMM_LENTGH + 1)
    return(1);

  n = sscanf(start_date_time, "%4d%3d%2d%2d%2d", &year, &mday, &h, &m, &s);
  if(n < 5)
    return(1);

  /* memset(&tm, 0, sizeof(tm)); */
  tm.tm_year = year - 1900;
  tm.tm_mon = 0;
  tm.tm_mday = mday;
  tm.tm_hour = h;
  tm.tm_min = m;
  tm.tm_sec = s;
  tm.tm_isdst = 0;	/* do not apply any (summer) time effect */

  /*
   * Let mktime do the conversions to the correct mon, mday.
   * Use timegm to interpret the input in UTC.
   */
  *seconds = timegm(&tm);

  n = strftime(ymd, YYYYDDMM_LENTGH + 1, "%Y%m%d", &tm);
  if(n != 0)
    n = strftime(hm, HHMM_LENTGH + 1, "%k%M", &tm);

  return(n != 0? 0 : 1);
}

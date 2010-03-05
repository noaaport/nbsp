/*
 * Copyright (c) 2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
/*
 * Returns:
 *
 * gribedition center subcenter model gridid reference_time [ftu p1 p2 tri]
 *
 * where
 *
 *	reference_time = yyyymmddhhMM
 *	ftu = forecast time unit (0 => minute, 1 => hour, ...)
 *	p1 = period in units of ftu (0 => analysis, ...)
 *	p2 = period (between analysis, forecasts, ...)
 *	tri = time range indicator (for the interpretation of p1,2)
 *
 * ftu, p1, p2, tri are returned only if the [-e] option is given.
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>	/* used by gribyear() */
#include <libgen.h>
#include "err.h"
#include "util.h"
#include "io.h"
#include "const.h"

#define WMO_SIZE		CTRLHDR_WMO_SIZE  /* includes \r\r\n */
#define IDENT_SECTION_SIZE	8
#define PDS_SECTION_SIZE	28

struct {
  char *opt_input_fname;
  int opt_background;
  int opt_extended_output; /* print ftu, p1, p2, tri */
  int opt_noheaders; /* [-l] start with GRIB (no headers at all) */
  int opt_hasgpk;    /* [-m] file has no ccb but has a gpk header */
  int opt_noccb;     /* [-n] file has no ccb[24] header (and no gpk header) */
  /* variables */
  FILE *input_fp;
} g = {NULL, 0, 0, 0, 0, 0, NULL};

static int process_file(void);
static void cleanup(void);
static unsigned short gribyear(unsigned char yy);

int main(int argc, char ** argv){

  int status = 0;
  int c;
  char *optstr = "belmn";
  char *usage = "nbspgrib [-b] [-e] [-l | -m | -n] file | < file";
  int opt_conflict_lmn = 0;

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'e':
      g.opt_extended_output = 1;
      break;
    case 'l':
      g.opt_noheaders = 1;
      ++opt_conflict_lmn;
      break;
    case 'm':
      g.opt_hasgpk = 1;		/* no ccb but gpk header */
      ++opt_conflict_lmn;
      break;
    case 'n':
      g.opt_noccb = 1;		/* the file has no ccb (and no gpk header) */
      ++opt_conflict_lmn;
      break;
    case 'h':
    default:
      status = 1;
      log_errx(1, usage);
      break;
    }
  }

  /* Conflicting options */
  if(opt_conflict_lmn > 1)
    log_errx(1, usage);

  if(optind < argc - 1)
    log_errx(1, "Too many arguments.");
  else if(optind == argc -1)
    g.opt_input_fname = argv[optind++];

  if(g.opt_background == 1)
    set_usesyslog();

  atexit(cleanup);

  if(status == 0)
    status = process_file();

  return(status != 0 ? 1 : 0);
}

static int process_file(void){

  int status = 0;
  int n;
  char header_buffer[WMO_SIZE + CCB_SIZE + GMPK_HEADER_SIZE]; 
  int header_size = WMO_SIZE + CCB_SIZE; /* the default */
  unsigned char section_buffer[IDENT_SECTION_SIZE + PDS_SECTION_SIZE];
  int section_size = IDENT_SECTION_SIZE + PDS_SECTION_SIZE;
  unsigned char *ids;	/* start of identification section */
  unsigned char *pds;	/* points to the start of the pds in the pds_buffer */
  unsigned char center, subcenter, model, gridid, gribedition;
  unsigned char yy, mm, dd, hh, MM;
  unsigned char ftu, p1, p2, tri; /* forecast time unit, periods, time range */
  unsigned short yyyy;

  /* Initialize once and for all */
  ids = &section_buffer[0];
  pds = &section_buffer[IDENT_SECTION_SIZE];

  if(g.opt_input_fname != NULL){
    g.input_fp = fopen(g.opt_input_fname, "r");
    if(g.input_fp == NULL)
      log_err_open(g.opt_input_fname);
  }else
    g.input_fp = stdin;

  if(g.opt_noccb == 1)
    header_size = WMO_SIZE;

  if(g.opt_hasgpk == 1)
    header_size = WMO_SIZE + GMPK_HEADER_SIZE;

  if(g.opt_noheaders == 1)
    header_size = 0;

  /* Read past the ccb or gpk and wmo headers */
  if(header_size > 0){
    n = read_page(g.input_fp, header_buffer, header_size);
    if(n < header_size){
      log_errx(1, "Corrupted wmo header.");
      return(1);
    }
  }

  n = read_page(g.input_fp, section_buffer, section_size);
  if(n < section_size){
    log_errx(1, "Corrupted section header.");
    return(1);
  }

  if((ids[0] != 'G') || (ids[1] != 'R') || (ids[2] != 'I') || (ids[3] != 'B')){
     log_errx(1, "GRIB marker not found.");
     return(1);
  }
  gribedition = ids[7];
  if(gribedition == 1){
    center = pds[4];
    subcenter = pds[25];
    model = pds[5];
    gridid = pds[6];
    yy = pds[12];
    yyyy = gribyear(yy);
    mm = pds[13];
    dd = pds[14];
    hh = pds[15];
    MM = pds[16];
    ftu = pds[17];
    p1 = pds[18];
    p2 = pds[19];
    tri = pds[20];
  }else{
    center = 0;
    subcenter = 0;
    model = 0;
    gridid = 0;
    yy = 0;
    yyyy = 0;
    mm = 0;
    dd = 0;
    hh = 0;
    MM = 0;
    ftu = 0;
    p1 = 0;
    p2 = 0;
    tri = 0;
  }
  fprintf(stdout, "%hhu %hhu %hhu %hhu %hhu",
	  gribedition, center, subcenter, model, gridid);

  fprintf(stdout, " %.2hu%.2hhu%.2hhu%.2hhu%.2hhu",
	  yyyy, mm, dd, hh, MM);

  if(g.opt_extended_output == 1)
    fprintf(stdout, " %hhu %hhu %hhu %hhu", ftu, p1, p2, tri);

  fprintf(stdout, "\n");

  return(status);
}

static void cleanup(void){

  if((g.input_fp != NULL) && (g.opt_input_fname != NULL))
    fclose(g.input_fp);
}

static unsigned short gribyear(unsigned char yy){
  /*
   * The grib1 files contain, in the reference time, the year relative
   * to the current century (e.g., 7, 12, 99). This function converts
   * to a 4-digit year.
   */
  time_t secs;
  struct tm *tm;
  int yyyy;

  secs = time(NULL);
  tm = gmtime(&secs);

  /*
   * Find how many multiples of 100 have passed and add the yy
   * from the grib file.
   */
  yyyy = ((1900 + tm->tm_year)/100)*100 + yy;

  return(yyyy);
}

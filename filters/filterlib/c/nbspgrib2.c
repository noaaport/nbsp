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
 *	gridid = 0
 *	reference_time = yyyymmddhhMM
 *	ftu = forecast time unit (indicator of unit time range)
 *	p1 = forecast time (in units of ftu)
 *      p2 and tri are returned as 0 for grib2.
 *
 * ftu, p1, p2, tri are returned only if the [-e] option is given.
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "err.h"
#include "util.h"
#include "io.h"
#include "const.h"

#define WMO_SIZE		CTRLHDR_WMO_SIZE  /* includes \r\r\n */
#define INDIC_SECTION_SIZE2	16

#define GRIB2_SECTION_HEADER_SIZE	5
#define GRIB2_SECTION_INIT_SIZE		(GRIB2_SECTION_HEADER_SIZE + 27)

struct grib2_section_st {
  void *buffer;
  int buffer_size;
  int size;
  int secnum;
};

struct {
  char *opt_input_fname;
  int opt_background;
  int opt_extended;  /* [-e] extended output to include forecast time */
  int opt_noheaders; /* [-l] start with GRIB (no headers at all) */
  int opt_hasgpk;    /* [-m] file has no ccb but has a gpk header */
  int opt_noccb;     /* [-n] file has no ccb[24] header (and no gpk header) */
  /* variables */
  FILE *input_fp;
} g = {NULL, 0, 0, 0, 0, 0, NULL};

static struct grib2_section_st *create_grib2_section(void);
static void destroy_grib2_section(struct grib2_section_st*);
static int read_grib2_section(FILE *fp, struct grib2_section_st* grb);
static int grow_grib2_section(struct grib2_section_st* grb, int newsize);

static int process_file(void);
static void cleanup(void);

int main(int argc, char ** argv){

  int status = 0;
  int c;
  char *optstr = "belmn";
  char *usage = "nbspgrib [-b] [-e] [-l |-m | -n] file | < file";
  int opt_conflict_lmn = 0;

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'e':
      g.opt_extended = 1;
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
  unsigned char indic_section_buffer[INDIC_SECTION_SIZE2];
  int indic_section_size = INDIC_SECTION_SIZE2;
  unsigned char *ind;	/* start of indicator section */
  unsigned char *ids;	/* points to the start of each section */
  unsigned short center, subcenter, model, gridid, gribedition;
  unsigned short yyyy, mm, dd, hh, MM;
  unsigned char ftu; /* forecast time unit (indicator of unit time range) */
  unsigned int  p1;  /* forecast time (in units of ftu) */
  unsigned char tri = 0; /* returned as 0 */
  unsigned int  p2 = 0; /* returned as 0 */
  struct grib2_section_st* grb;

  gribedition = 0;
  center = 0;
  subcenter = 0;
  model = 0;
  gridid = 0;
  yyyy = 0;
  mm = 0;
  dd = 0;
  hh = 0;
  MM = 0;
  ftu = 0;
  p1 = 0;
  p2 = 0;
  tri = 0;

  /* Initialize once and for all */
  ind = &indic_section_buffer[0];

  grb = create_grib2_section();
  if(grb == NULL){
    log_err(1, "Cannot initialize create_grib2_section()");
    return(1);
  }

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

  /* Read the indicator section */
  n = read_page(g.input_fp, ind, indic_section_size);
  if(n < indic_section_size){
    log_errx(1, "Corrupted Indicator section header.");
    return(1);
  }

  if((ind[0] != 'G') || (ind[1] != 'R') || (ind[2] != 'I') || (ind[3] != 'B')){
     log_errx(1, "GRIB marker not found.");
     return(1);
  }
  gribedition = ind[7];

  if(gribedition != 2){
    fprintf(stdout, "%hu %hu %hu %hu %hu",
	    gribedition, center, subcenter, model, gridid);
    fprintf(stdout, " %.2hu%.2hu%.2hu%.2hu%.2hu\n",
	    yyyy, mm, dd, hh, MM);

    return(0);
  }

  /* Read section by section */
  while(status == 0){
    status = read_grib2_section(g.input_fp, grb);
    if(status != 0)
      break;
    
    ids = (unsigned char*)grb->buffer;
    if(grb->secnum == 1){
      center = (ids[5] << 8) + ids[6];
      subcenter = (ids[7] << 8) + ids[8];
      yyyy = (ids[12] << 8) + ids[13];
      mm = ids[14];
      dd = ids[15];
      hh = ids[16];
      MM = ids[17];
    }else if(grb->secnum == 4){
      model = ids[13];
      ftu = ids[17];
      p1 = (ids[18] << 24) + (ids[19] << 16) + (ids[20] << 8) + ids[21];
      break;
    }
  }

  fprintf(stdout, "%hu %hu %hu %hu %hu",
	  gribedition, center, subcenter, model, gridid);
  fprintf(stdout, " %.2hu%.2hu%.2hu%.2hu%.2hu",
	  yyyy, mm, dd, hh, MM);

  if(g.opt_extended == 1)
    fprintf(stdout, " %hhu %u %u %hhu", ftu, p1, p2, tri);

  fprintf(stdout, "\n");

  destroy_grib2_section(grb);

  return(status);
}

static void cleanup(void){

  if((g.input_fp != NULL) && (g.opt_input_fname != NULL))
    fclose(g.input_fp);
}

/*
 * Fuctions to support reading grib2 sections.
 */
static struct grib2_section_st *create_grib2_section(void){

  struct grib2_section_st *grb;
  int status = 0;

  grb = malloc(sizeof(struct grib2_section_st));
  if(grb == NULL)
    status = -1;

  if(status == 0){
    grb->buffer = NULL;
    grb->buffer_size = GRIB2_SECTION_INIT_SIZE;
    grb->size = 0;
    grb->secnum = 0;

    grb->buffer = malloc(GRIB2_SECTION_INIT_SIZE);
    if(grb->buffer == NULL){
      free(grb);
      grb = NULL;
      status = -1;
    }
  }

  if(status != 0)
    log_err(1, "Cannot create grib2_section_st.");

  return(grb);
}
  
static void destroy_grib2_section(struct grib2_section_st* grb){

  if(grb->buffer != NULL)
    free(grb->buffer);

  free(grb);
}

static int read_grib2_section(FILE *fp, struct grib2_section_st* grb){

  unsigned char *p;
  int size;
  int secnum;
  int n;

  p = (unsigned char*)(grb->buffer);
  n = read_page(fp, p, GRIB2_SECTION_HEADER_SIZE);
  if(n != GRIB2_SECTION_HEADER_SIZE){
    log_errx(1, "Corrupt section header.");
    return(1);
  }

  size = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
  secnum = p[4];
  
  if((size > grb->buffer_size) && (grow_grib2_section(grb, size) != 0))
    return(-1);

  p = &((unsigned char*)(grb->buffer))[GRIB2_SECTION_HEADER_SIZE];
  n = read_page(fp, p, size - GRIB2_SECTION_HEADER_SIZE);
  if(n != size - GRIB2_SECTION_HEADER_SIZE){
    log_errx(1, "Corrupt section header.");
    return(1);
  }
  
  grb->size = size;
  grb->secnum = secnum;

  return(0);
}

static int grow_grib2_section(struct grib2_section_st* grb, int newsize){

  void *p;

  p = realloc(grb->buffer, newsize);
  if(p == NULL){
    log_err(1, "Cannot grow grib2_section_st buffer.");
    return(-1);
  }

  grb->buffer = p;
  grb->buffer_size = newsize;

  return(0);
}

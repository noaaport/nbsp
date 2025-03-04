/*
 * Copyright (c) 2025 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 *
 * Usage:
 * nbspgoesrasc [-b] [-e <inputstr>] [-f <inputfile>] [-n <basename>] <ascfile>
 *
 * -b => background
 * -e => input string, for example "-70,14,-60,24,1" (conflicts with -f)
 * -f => input strings read from the file (conflicts with -e)
 * -n => basename for the output files (default is "output")
 *
 * Example:
 *
 * Given a file like
 *
 * OR_ABI-L1b-RadF-M6C01_G16_s20243411710207_e20243411719515_c20243411719563.nc
 *
 * (call it <file>.nc) then first create the asc file
 *
 *   nbspgoesrgis -a <file<.asc -r <file>.nc
 *
 * The -r option tells nbspgoesrgis that the nc file is an OR_ABI file
 * rather than a noaaport tixxnn file. Then to "cut" the asc file
 *
 *   nbspgoesrasc -e "-70,12,-60,22,1" <file>.asc
 *
 * will produce the file "output001.asc" with the coordinates limited
 * by the rectangle "-70,12,-60,22". The "1" at the end defines the
 * the index of the output file name. In addition the [-n] can be
 * used to set the basename of the output; for example
 *
 *   nbspgoesrasc -e "-70,12,-60,22,9" -n zone <file>.asc
 *
 * will produce the file with the same contents, but named "zone009.asc".
 */
/*
 * The fixed parameters are:
 *
 * nx, ny,
 * xll, yll
 * cellsize
 *
 * The input parameters for each cut are
 *
 * xmin, xmax, ymin, ymax
 *
 * and the corrresponding return values for each cut are
 *
 * i1, j1, i2, j2,
 * nnx, nny		(the number of points of the new box)
 *
 */
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include "err.h"

/* If FNAMEFMT is changed, check fname_length in the function init() */
#define DCGOESR_GRID_MAP_NODATA -1
#define DCGOESRASC_OUTPUT_BASENAME "output"
#define DCGOESRASC_OUTPUT_FNAMEFMT "%s%03d%s"
#define DCGOESRASC_OUTPUT_SUFFIX ".asc"

struct cutasc_st {
  size_t nx;
  size_t ny;
  double xll;
  double yll;
  double cellsize;
  /* The inputs of the new box */
  double xmin;
  double ymin;
  double xmax;
  double ymax;
  /* The calculated indices of the nex box */
  size_t i1;
  size_t j1;
  size_t i2;
  size_t j2;
  size_t nnx;	/* number of points of the new box */
  size_t nny;	/* number of points of the new box */
};

struct {
  int opt_background;		/* -b */
  char *opt_inputstr;		/* -e */
  char *opt_inputfile;		/* -f */
  char *opt_basename;           /* -n */
  char *opt_ascfile;		/* the input asc data file */
  /* variables */
  char *outputfile;
  int outfname_length;		/* length of the output file name */
  struct cutasc_st *ca;
  int *data;
  FILE *infp;			/* fp of input file (-f option) */
} g = {0, NULL, NULL, DCGOESRASC_OUTPUT_BASENAME, NULL,
       NULL, 0, NULL, NULL, NULL};

static void init(void);
static void cleanup(void);
static void load_data(void);
static int cutasc_write_data(FILE *fp);
static int process_str(char *str);
static int process_file(char *file);
static int process_input(int index);

static void init(void) {

  int fname_length;
  
  g.data = NULL;
  g.infp = NULL;

  g.ca = malloc(sizeof(struct cutasc_st));
  if(g.ca == NULL)
    log_err(1, "%s", "Error from malloc");

  /* The "3" depends on the DCGOESRASC_OUTPUT_FNAMEFMT */
  fname_length = strlen(g.opt_basename) + 3 + strlen(DCGOESRASC_OUTPUT_SUFFIX);
  g.outputfile = malloc(fname_length + 1);
  if(g.outputfile == NULL)
    log_err(1, "%s", "Error from malloc");

  g.outfname_length = fname_length;
}

static void cleanup(void) {

  if(g.data != NULL)
    free(g.data);

  if(g.infp != NULL)
    fclose(g.infp);

  if(g.ca != NULL)
    free(g.ca);

  if(g.outputfile != NULL)
    free(g.outputfile);
}

int main(int argc, char **argv){

  char *optstr = "be:f:n:";
  char *usage = "nbspgoesrasc [-b] [-e <inputstr>] [-f <inputfile>]"
    " [-n <basename>] <ascfile>";
  int status = 0;
  int c;

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'e':
      g.opt_inputstr = optarg;
      break;
    case 'f':
      g.opt_inputfile = optarg;
      break;
    case 'n':
      g.opt_basename = optarg;
      break;      
    default:
      log_info(usage);
      exit(0);
      break;
    }
  }

  if(g.opt_background == 1)
    set_usesyslog();

  if((g.opt_inputstr != NULL) && (g.opt_inputfile != NULL))
    log_errx(1, "%s", "Options -e and -f conflict.");
  else if((g.opt_inputstr == NULL) && (g.opt_inputfile == NULL))
    log_errx(1, "%s", "One of -e and -f must be given.");
  
  if(optind < argc - 1)
    log_errx(1, "Too many arguments.");
  else if(optind > argc -1)
    log_errx(1, "Requires one argument.");
  
  g.opt_ascfile = argv[optind++];	/* the input asc file */

  init();
  atexit(cleanup);

  load_data();

  if(g.opt_inputstr != NULL)
    status = process_str(g.opt_inputstr);
  else {
    status = process_file(g.opt_inputfile);
  }

  return(status != 0 ? 1 : 0);
}

static void load_data(void) {

  FILE *fp;
  size_t nx, ny;
  size_t npoints;
  double x1, x2, x3;
  int nodata;
  size_t k;
  int status = 0;

  fp = fopen(g.opt_ascfile, "r");
  if(fp == NULL)
    log_err(1, "Cannot open %s", g.opt_ascfile);
  
  if(fscanf(fp, "ncols %zu nrows %zu", &nx, &ny) != 2)
    status = 1;

  if(status == 0) {
    if(fscanf(fp, " xllcorner %lf yllcorner %lf cellsize %lf nodata_value %d",
	      &x1, &x2, &x3, &nodata) != 4)
      status = 1;
  }

  if(status != 0) {
    fclose(fp);
    log_err(1, "Error reading %s", g.opt_ascfile);
  }
  
  npoints = nx * ny;
  g.data = malloc(sizeof(*g.data) * npoints);
  if(g.data == NULL) {
    fclose(fp);
    log_err(1, "%s", "Error from malloc");
  }
  
  for(k = 0; k < npoints; ++k) {
    if(fscanf(fp, " %d", &g.data[k]) != 1) {
      status = 1;
      break;
    }
  }
 
  fclose(fp);
  if(status != 0)
    log_err(1, "Error reading %s", g.opt_ascfile);
 
  /* fprintf(stdout, "%zu %zu %f %f %f %d\n", nx, ny, x1, x2, x3, nodata); */

  /* fill in the ca struct */
  g.ca->nx = nx;
  g.ca->ny = ny;
  g.ca->xll = x1;
  g.ca->yll = x2;
  g.ca->cellsize = x3;
}

static int cutasc_write_data(FILE *fp) {
  /*
   * Output the data in asc format.
   *
   * ncols 157
   * nrows 171
   * xllcorner -156.08749650000
   * yllcorner 18.870890200000
   * cellsize 0.00833300
   * nodata_value -99     (optional)
   * 0 0 1 1 1 2 3 3 5 6 8 9 12 14 18 21 25 30 35 41 47 53
   * 59 66 73 79 86 92 97 102 106 109 112 113 113 113 111 109 106
   * 103 98 94 89 83 78 72 67 61 56 51 46 41 37 32 29 25 22 19
   * etc...
   */
  size_t i, j, k;
  int n;
  int c;

  n = fprintf(fp, "ncols %zu\n", g.ca->nnx);
  if(n > 0)
    n = fprintf(fp, "nrows %zu\n", g.ca->nny);

  if(n > 0)
    n = fprintf(fp, "xllcorner %f\n", g.ca->xmin);

  if(n > 0)
    n = fprintf(fp, "yllcorner %f\n", g.ca->ymin);

  if(n > 0)
    n = fprintf(fp, "cellsize %f\n", g.ca->cellsize);

  if(n > 0)
    n = fprintf(fp, "nodata_value %d\n", DCGOESR_GRID_MAP_NODATA);

  if(n < 0)
    return(-1);

  for(j = g.ca->j1; j <= g.ca->j2; ++j){
    c = ' ';
    for(i = g.ca->i1; i <= g.ca->i2; ++i){
      if(i == g.ca->i2)
	c = '\n';

      k = j*g.ca->nx + i;
      if(fprintf(fp, "%d%c", g.data[k], c) < 0)
	return(-1);
    }
  }

  return(0);
}

static int process_str(char *str) {

  int index;
  int status = 0;

  if(sscanf(str, "%lf,%lf,%lf,%lf,%d",
	   &g.ca->xmin, &g.ca->ymin, &g.ca->xmax, &g.ca->ymax, &index) != 5) {
    log_errx(1, "Incomplete input string: %s", str);
  }

  status = process_input(index);

  return(status);
}

static int process_input(int index) {
  
  double xur, yur;
  int n;
  FILE *fp;
  int status = 0;

  /* the coordinates of the ur point */
  xur = g.ca->xll + g.ca->cellsize * g.ca->nx;
  yur = g.ca->yll + g.ca->cellsize * g.ca->ny;

  if((g.ca->xmin < g.ca->xll) ||
     (g.ca->ymin < g.ca->yll) ||
     (g.ca->xmax > xur) ||
     (g.ca->ymax > yur)) {
    log_errx(1, "Box exceeds original: %f,%f,%f,%f",
	     g.ca->xmin, g.ca->ymin, g.ca->xmax, g.ca->ymax);
  }
  
  /*
   * Calculate the indices of the new box.
   */   
  /* the calculated indices */
  g.ca->i1 = (int)((g.ca->xmin - g.ca->xll)/g.ca->cellsize);
  g.ca->j1 = (int)((yur - g.ca->ymax)/g.ca->cellsize);
  
  g.ca->i2 = (int)((g.ca->xmax - g.ca->xll)/g.ca->cellsize);
  g.ca->j2 = (int)((yur - g.ca->ymin)/g.ca->cellsize);

  g.ca->nnx = g.ca->i2 - g.ca->i1 + 1;
  g.ca->nny = g.ca->j2 - g.ca->j1 + 1;

  /* Open the output file */
  n = snprintf(g.outputfile, g.outfname_length + 1, "%s%03d%s",
	       g.opt_basename, index, DCGOESRASC_OUTPUT_SUFFIX);
  assert(n == g.outfname_length);

  fp = fopen(g.outputfile, "w");
  if(fp == NULL)
    log_err(1, "Cannot open %s", g.outputfile);

  status = cutasc_write_data(fp);
  if(status != 0) {
    log_err(0, "Error writing to %s", g.outputfile);
  }

  fclose(fp);

  return(status);
}


static int process_file(char *file) {

  int index;
  int n;
  int status = 0;

  fprintf(stdout, "%s\n", file);

  g.infp = fopen(file, "r");
  if(g.infp == NULL)
    log_err(1, "Cannot open %s", file);

  do {
    n = fscanf(g.infp, "%lf,%lf,%lf,%lf,%d ",
	       &g.ca->xmin, &g.ca->ymin, &g.ca->xmax, &g.ca->ymax, &index);
    if(n == 5)
      status = process_input(index);
    else
      status = 1;

  } while(status == 0);

  fclose(g.infp);

  if((status != 0) && (n != EOF))
    log_errx(1, "Incomplete input string");
  
  return(status);
}

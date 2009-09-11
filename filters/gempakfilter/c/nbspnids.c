/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include "err.h"

/*
 * NOTE: The input file to this program can be the compressed or the
 * uncompressed file (gpmap_gif accepts either), but
 * without the ccb (this is what gpmap_gif expects).
 */

#define GPMAP_GIF_BIN	"gpmap_gif"
#define OUTEXT		"gif"

static struct {
  char *opt_gpmap_gif;
  char *opt_input_fname;
  char *opt_output_fname;
  char *opt_size;
  char *opt_map;
  char *opt_mapfil;
  char *opt_imcbar;
  int opt_verbose;
  int opt_background;
  /* variables */
  char *ext;
} g = {GPMAP_GIF_BIN, NULL, NULL, "800;600", "1", "states + county", "1", 0, 0,
       OUTEXT};

static int process_file(void);
static void cleanup(void);

static void cleanup(void){

  unlink("gemglb.nts");
  unlink("last.nts");
}

int main(int argc, char **argv){

  int status = 0;
  char *optstr = "hbf:i:m:o:q:s:v";
  char *usage = "nbspnids [-b] [-f mapfil] [-i imcbar] [-m map] [-o outfile] "
    "[-q gpmap_gif_bin] [-s size] [-v]";
  int c;

  atexit(cleanup);

  set_progname(basename(argv[0]));

 while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'f':
      g.opt_mapfil = optarg;
      break;
    case 'i':
      g.opt_imcbar = optarg;
      break;
    case 'm':
      g.opt_map = optarg;
      break;
    case 'o':
      g.opt_output_fname = optarg;
      break;
    case 'q':
      g.opt_gpmap_gif = optarg;
      break;
    case 's':
      g.opt_size = optarg;
      break;
    case 'v':
      g.opt_verbose = 1;
      break;
    case 'h':
    default:
      status = 1;
      log_info(usage);
      exit(0);
      break;
    }
  }

  if(optind != argc - 1)
    log_errx(1, "Needs one argument.");

  g.opt_input_fname = argv[optind];

  if(g.opt_background == 1)
    set_usesyslog();

  if(status == 0)
    status = process_file();

  return(status != 0 ? 1 : 0);
}

static int process_file(void){

  int status = 0;
  FILE *fp = NULL;
  size_t n;

  if(g.opt_output_fname == NULL){
    n = strlen(g.opt_input_fname) + strlen(g.ext) + 2;
    g.opt_output_fname = malloc(n);

    if(g.opt_output_fname == NULL)
      log_err(1, "Memory error.");
  
    if((size_t)snprintf(g.opt_output_fname, n, "%s.%s",
			g.opt_input_fname, g.ext) != n - 1){
      log_errx(1, "Miscalculation of output_fname size.");
    }
  }

  if(g.opt_verbose == 0)
    close(STDOUT_FILENO);

  fp = popen(g.opt_gpmap_gif, "w");
  if(fp == NULL)
    log_errx(1, "Cannot open %s", g.opt_gpmap_gif);

  fprintf(fp, "$mapfil = %s\n", g.opt_mapfil);
  fprintf(fp, "map = %s\n", g.opt_map);
  fprintf(fp, "device = %s|%s|%s\n", g.ext, g.opt_output_fname, g.opt_size);
  fprintf(fp, "radfil = %s\n", g.opt_input_fname);
  fprintf(fp, "imcbar = %s\n", g.opt_imcbar);

  fprintf(fp, "garea = dset\n");
  fprintf(fp, "lut = default\n");
  fprintf(fp, "proj = rad\n");
  fprintf(fp, "latlon = 0\n");
  fprintf(fp, "r\n\ne\n");

  pclose(fp);

  return(status);
}

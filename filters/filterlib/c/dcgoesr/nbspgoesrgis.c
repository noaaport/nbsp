/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 *
 * Usage: nbspgoesrgis [-b] <ncfile>
 *
 * -b => bakground
  */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> /* getopt */
#include <libgen.h> /* basename */
#include <err.h>
#include "err.h"
#include "dcgoesr.h"

struct {
  int opt_background;		/* -b */
  /* variables */
  char *opt_inputfile;
} g = {0, NULL};

/* static functions */
static void init(void);
static void cleanup(void);

int main(int argc, char **argv) {

  char *optstr = "bh";
  char *usage = "nbspgoesrgis [-h] [-b] <inputfile>";
  int c;
  int status = 0;

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
  
  return(0);
}

static void init(void) {

  ;
}

static void cleanup(void) {

  ;
}

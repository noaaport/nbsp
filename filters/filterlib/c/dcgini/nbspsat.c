/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

/*
 * Usage:
 *
 *  nbspsat [-C | -g | -i | -n] [options] [file | < file]
 *
 * -C => check pdb header and exit
 * -i => print info and exit
 * -g => gempak output (compress file)
 * -n => png image output (default)
 *
 * The program can read from a file or from stdin, but it assumes that
 * the input is the uncompressed gini data file. Only when it is
 * invoked with [-i] to just extract the relevant info,
 * it can take either the compressed or uncompresed file as input.
 * It prints to stdout
 *
 *	npdb->source, 
 *	npdb->creating_entity, 
 *	npdb->sector, 
 *	npdb->channel,
 *	npdb->res,
 *	time, (date-time string)
 *	fname
 *
 * With the [-e] option it prints the following additional values after those:
 *
 *	map_projection (map projection indicator)
 *	proj_center_flag (octet 37)
 *	scan_mode	 (octet 38)
 *	lat1  (lat of first grid point) int x 10000
 *	lon1  (lon of first grid point) int x 10000
 *      lov   (orientation of grid)     int x 10000
 *      dx    (x-direction increment)
 *      dy    (y-direction increment)
 *	nx    (number of points in x direction)
 *	ny    (number of points in y direction)
 *	res   (same thing as npdb->res; duplicated for convenience)
 *      latin (Earth tangent)           int x 10000
 *	lat2  (lat of last grid point) int x 10000
 *	lon2  (lon of last grid point) int x 10000
 *      lat_ur  (upper right-hand corner lat) int x 10000
 *      lon_ur  (upper right-hand corner lon) int x 10000
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <png.h>
#include <zlib.h>
#include <libgen.h>
#include "err.h"
#include "io.h"
#include "unz.h"
#include "dcgini_misc.h"
#include "dcgini_name.h"
#include "dcgini_pdb.h"
#include "dcgini_util.h"

struct {
  char *opt_inputfile;
  char *opt_output_dir;		/* -d */
  char *opt_fname_prefix;	/* -p => change the default prefix name */
  char *opt_output_fname;	/* -o */
  int opt_C;			/* -C => check pdb */
  int opt_silent;		/* -s */
  int opt_gempak_output;	/* -g */
  int opt_png_output;	        /* -n => default if none specified */
  int opt_background;		/* -b */
  int opt_wrinfo;		/* -i => only extract and print info */
  int opt_extended_info;        /* -e => extended info output (when -i) */
  /*
   * variables
   */
  int fd;
} g = {NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, -1};

static void cleanup(void);
static int write_pngdata(FILE *fp, unsigned char *data,
			 int linesize, int numlines);
static int write_gempak(int gempak_fd, int fd,
       struct nesdis_pdb_st *npdb, int *unz_status);
static int process_file(void);
static int write_file_info(void);
static void check(struct nesdis_pdb_st *npdb);
static void output(char *fname,
		   struct nesdis_pdb_st *npdb,
		   int option_extended_info);

int main(int argc, char **argv){

  int status = 0;
  int c;
  char *optstr = "hCbd:egino:p:s";
  char *usage = "nbspsat [-C|-i|-g|-n] [-b] [-o name] [-d outputdir] [-e]"
    " [-p prefix] [-s] [file | < file]";
  int conflict_Cign = 0;
  int conflict_is = 0;
  int conflict_op = 0;

  set_progname(basename(argv[0]));
  atexit(cleanup);

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'C':
      ++conflict_Cign;
      g.opt_C = 1;
      break;
    case 'b':
      g.opt_background = 1;
      break;
    case 'd':
      g.opt_output_dir = optarg;
      break;
    case 'e':
      g.opt_extended_info = 1;
      break;
    case 'g':
      ++conflict_Cign;
      g.opt_gempak_output = 1;
      break;
    case 'i':
      ++conflict_Cign;
      ++conflict_is;
      g.opt_wrinfo = 1;
      break;
    case 'n':
      ++conflict_Cign;
      g.opt_png_output = 1;
      break;
    case 'o':
      ++conflict_op;
      g.opt_output_fname = optarg;
      break;
    case 'p':
      ++conflict_op;
      g.opt_fname_prefix = optarg;
      break;
    case 's':
      ++conflict_is;
      g.opt_silent = 1;
      break;
    case 'h':
    default:
      log_info(usage);
      exit(0);
      break;
    }
  }

  if(conflict_Cign > 1)
    log_errx(1, "Conflicting options: Cign. %s", usage);
  else if(conflict_Cign == 0){
    /* The default action is to create the png image file */
    g.opt_png_output = 1;
  }

  if(conflict_is > 1)
    log_errx(1, "Conflicting options: is. %s", usage);
  
  if(conflict_op > 1)
    log_errx(1, "Conflicting options: op. %s", usage);

  if(optind < argc - 1)
    log_errx(1, "Too many arguments.");
  else if(optind == argc - 1)
    g.opt_inputfile = argv[optind];

  if(g.opt_background == 1)
    set_usesyslog();

  if(g.opt_wrinfo == 1)
    status = write_file_info();
  else
    status = process_file();

  return(status != 0 ? 1 : 0);
}

static void cleanup(void){

  if((g.fd != -1) && (g.opt_inputfile != NULL))
    (void)close(g.fd);
}

static int process_file(void){

  int fd;
  struct nesdis_pdb_st npdb;
  unsigned char *data = NULL;	/* records data (after pdb) */
  size_t data_size;		/* linesize * numlines */
  int n;
  int gempak_fd;
  FILE *fp;
  int status = 0;
  int zstatus = 0;
  char *fname;

  /* Initialize */
  npdb.buffer_size = NESDIS_WMO_HEADER_SIZE + NESDIS_PDB_SIZE;

  if(g.opt_inputfile == NULL)
    fd = fileno(stdin);
  else{
    fd = open(g.opt_inputfile, O_RDONLY);
    if(fd == -1)
      log_err_open(g.opt_inputfile);
    else
      g.fd = fd;
  }

  status = read_nesdis_pdb(fd, &npdb);
  if(status != 0){
    if(g.opt_inputfile != NULL)
      log_warnx("Error reading nesdis header from %s", g.opt_inputfile);

    if(status == -1)
      log_err(1, "Error from read_nesdis_pdb()");
    else if(status == 1)
      log_errx(1, "File too short");
    else if(status == 2)
      log_errx(1, "File has invalid wmo header");
  }

  if(g.opt_C == 1){
    check(&npdb);
    return(0);
  }

  if(g.opt_png_output == 1){
    /*
     * Read the data once and for all
     */
    data_size = npdb.linesize * npdb.numlines;
    data = malloc(data_size);
    if(data == NULL)
      log_err(1, "malloc()");

    /* Use readf in case the input comes from a pipe (stdin) */
    n = readf(fd, data, data_size);
    if(n == -1)
      log_err(1, "Error from read()");
    else if((size_t)n != data_size)
      log_errx(1, "File is corrupt (short)");
  }

  if(g.opt_output_dir != NULL){
    status = chdir(g.opt_output_dir);
    if(status != 0)
      log_err(1, "Cannot chdir to %s", g.opt_output_dir);
  }

  if(g.opt_output_fname == NULL){
    fname = dcgini_default_name(&npdb, NULL, DCGINI_PNGEXT);
    if(fname == NULL)
      log_err(1, "malloc()");
  }else{
    fname = malloc(strlen(g.opt_output_fname) + 1);
    if(fname == NULL)
      log_err(1, "malloc");

    sprintf(fname, "%s", g.opt_output_fname);
  }

  if(g.opt_png_output == 1){
    fp = fopen_output(fname, "w");
    if(fp == NULL)
      log_err(1, "Could not open %s", fname);
    
    if((status = write_pngdata(fp, data, npdb.linesize, npdb.numlines)) != 0){
      unlink(fname);
      if(status == -1)
	log_err(1, "Could not write to %s", fname);
      else
	log_errx(1, "Could not write to %s: error from libpng", fname);
    }
    fclose(fp);
  }

  if(g.opt_gempak_output == 1){
    /* If fname is the default (png name), remove the extension */
    if(g.opt_output_fname == NULL){
      fname[strlen(fname) - 4] = '\0';
    }
    gempak_fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(gempak_fd == -1)
      log_err(1, "Could not open %s", fname);

    if((status = write_gempak(gempak_fd, fd, &npdb, &zstatus)) != 0){
      unlink(fname);
      if(status == -1)
	log_err(1, "Could not write to %s", fname);
      else if(status == 1)
	log_errx(1, "zip error %d compressing %s", zstatus, fname);
    }
    close(gempak_fd);
  }

  if(g.opt_silent == 0)
    output(fname, &npdb, g.opt_extended_info);

  if(data != NULL)
    free(data);

  free(fname);

  return(0);
}

static int write_pngdata(FILE *fp, unsigned char *data,
			 int linesize, int numlines){
  
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  int bit_depth = 8;
  int color_type = PNG_COLOR_TYPE_GRAY;
  int interlace_type = PNG_INTERLACE_NONE;
  int compression_type = PNG_COMPRESSION_TYPE_DEFAULT;
  int filter_method = PNG_FILTER_TYPE_DEFAULT;
  int width = linesize;
  int height = numlines;
  unsigned char *row;
  int i;
  volatile int status = 0;   /*
			      * warning: variable 'status' might be
			      * clobbered by `longjmp'
			      */

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(png_ptr != NULL){
    info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == NULL){
      png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
      png_ptr = NULL;
    }
  }

  if(png_ptr == NULL){
    status = -1;
    goto end;
  }

  if(setjmp(png_jmpbuf(png_ptr))){
    status = 1;		/* error from png lib */
    goto end;
  }

  png_init_io(png_ptr, fp);
  png_set_IHDR(png_ptr, info_ptr, width, height,
	       bit_depth, color_type, interlace_type,
	       compression_type, filter_method);
  png_write_info(png_ptr, info_ptr);

  row = data;
  for(i = 1; i <= numlines; ++i){
    png_write_row(png_ptr, row);
    row += linesize;
  }

 end:

  if(status == 0)
    png_write_end(png_ptr, info_ptr);
  
  if(png_ptr != NULL)
    png_destroy_write_struct(&png_ptr, &info_ptr);

  return(status);
}

static int write_gempak(int gempak_fd, int fd,
			struct nesdis_pdb_st *npdb, int *zstatus){
  /*
   * Returns:
   *  0 => no errors
   * -1 => system error
   *  1 => error from zip
   *  2 => the file ended before reading all the npdb->numlines.
   */
  int status = 0;  
  char *block = NULL;
  char *zblock = NULL;
  int block_size = NESDIS_DATA_BLOCKSIZE;
  int zblock_size;
  int level = 9;
  int n;

  if(lseek(fd, npdb->buffer_size, SEEK_SET) == -1)
    return(-1);

  if(write(gempak_fd, npdb->buffer, NESDIS_WMO_HEADER_SIZE) == -1)
    return(-1);

  *zstatus = zip(&zblock, &zblock_size, npdb->buffer, npdb->buffer_size,
		 level);
  if(*zstatus == 0){
    if(write(gempak_fd, zblock, zblock_size) == -1)
      status = -1;

    free(zblock);
  }else
    status = 1;

  if(status != 0)
    return(status);

  block = malloc(block_size);
  if(block == NULL)
    return(-1);

  while((n = read(fd, block, block_size)) > 0){
    *zstatus = zip(&zblock, &zblock_size, block, n, level);
    if(*zstatus != 0){
      status = 1;
      break;
    }

    if(write(gempak_fd, zblock, zblock_size) == -1)
      status = -1;

    free(zblock);

    if(status != 0)
      break;
  }

  if(n == -1)
    status = -1;

  free(block);

  return(status);
}

static int write_file_info(void){
  /*
   * This function extracts and prints (to stdout) the information
   * from the nesdis pdb that is used by the filters. It tries to
   * detect if the input file is compresed (in the gempak-like form
   * with a wmo and the compressed frames appended one after the other.
   * If the file is compressed, then there is a plain text wmo. Then 
   * comes the first compressed frame, which contains the wmo
   * and the pdb after it is uncompressed.
   */
  int fd;
  struct nesdis_pdb_st npdb;
  char *output_fname;
  int status = 0;

  if(g.opt_inputfile == NULL)
    fd = fileno(stdin);
  else{
    fd = open(g.opt_inputfile, O_RDONLY);
    if(fd == -1)
      log_err_open(g.opt_inputfile);
    else
      g.fd = fd;
  }

  status = read_nesdis_pdb_compressed(fd, &npdb);

  if(status != 0){
    if(g.opt_inputfile != NULL)
      log_warnx("Error reading nesdis header from %s", g.opt_inputfile);

    if(status == -1){
      log_err(1, "Error from read()");
      return(-1);
    }else if(status != 0){
      if(status == 1)
	log_errx(1, "File too short");
      else if(status == 2)
	log_errx(1, "File has invalid wmo header");
      else
	log_errx(1, "Could not read nesdis pdb. Error from zlib.");
      
      return(1);
    }
  }

  output_fname = dcgini_default_name(&npdb, NULL, DCGINI_PNGEXT);
  if(output_fname == NULL)
    log_err(1, "Error from malloc().");

  output(output_fname, &npdb, g.opt_extended_info);
  free(output_fname);

  return(0);
}

static void check(struct nesdis_pdb_st *npdb){
  
  fprintf(stdout, "%d%02d%02d_%02d%02d\n", 
	  npdb->year, npdb->month, npdb->day, npdb->hour, npdb->min);

  fprintf(stdout, "%d %d %d\n", npdb->numlines, npdb->linesize,
	  (npdb->numlines + 1) * npdb->linesize + NESDIS_WMO_HEADER_SIZE + 
	  NESDIS_PDB_SIZE);

  fprintf(stdout, "nx, ny: %d %d\n", npdb->nx, npdb->ny);

  fprintf(stdout, " source: %d\n", npdb->source);
  fprintf(stdout, " entity: %d\n", npdb->creating_entity);
  fprintf(stdout, " sector: %d\n", npdb->sector);
  fprintf(stdout, "channel: %d\n", npdb->channel);
  fprintf(stdout, "    res: %d\n", npdb->res);
}

static void output(char *fname,
		   struct nesdis_pdb_st *npdb,
		   int option_extended_info){

  fprintf(stdout, "%d %d %d %d %d " DCGINI_DEFAULT_TIME_FMT " %s",
	  npdb->source, 
	  npdb->creating_entity, 
	  npdb->sector, 
	  npdb->channel,
	  npdb->res,
	  npdb->year, npdb->month, npdb->day, npdb->hour, npdb->min,
	  fname);

  if(option_extended_info == 0){
    fprintf(stdout, "\n");
    return;
  }

  fprintf(stdout, " %d %d %#x %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
	  npdb->map_projection,
	  npdb->proj_center_flag,
	  npdb->scan_mode,
	  npdb->lat1,
	  npdb->lon1,
	  npdb->lov,
	  npdb->dx,
	  npdb->dy,
	  npdb->nx,
	  npdb->ny,
	  npdb->res,
	  npdb->latin,
	  npdb->lat2,
	  npdb->lon2,
	  npdb->lat_ur,
	  npdb->lon_ur);
}

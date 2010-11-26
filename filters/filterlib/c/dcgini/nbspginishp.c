/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
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
#include "dcgini_pdb.h"
#include "dcgini_util.h"

/*
 * Usage: nbspginishp [output options] <file> | < <file>
 *
 * The program reads from a file or stdin, but the data must 
 * be the uncompressed gini data. Only when it is invoked with [-i]
 * to just extract the relevant info, it can take either the compressed
 * or uncompresed file as input. The output options are:
 *
 *  -a => do them all with the default names
 *  -v <csv file>
 *  -p <shp file>
 *  -x <shx file>
 *  -f <dbf file>
 *  -o <info file>
 */

struct {
  char *opt_output_dir;		/* -d */
  int opt_background;		/* -b */
  int opt_all;			/* -a */
  int opt_wrinfo;		/* -i => only extract and print info */
  int opt_extended_info;        /* -e => extended info output (when -i) */
  int opt_silent;		/* -s */
} g = {NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0};

#define NBSPSAT_DEFAULT_NAME_FMT "%s_%d%02d%02d_%02d%02d"
#define NBSPSAT_DEFAULT_PNGNAME_FMT NBSPSAT_DEFAULT_NAME_FMT ".png"

static int write_pngdata(FILE *fp, unsigned char *data,
			 int linesize, int numlines);
static int write_gempak(int gempak_fd, int fd,
       struct nesdis_pdb *npdb, int *unz_status);
static int process_file(char *in_file);
static int write_file_info(char *in_file);
static void check(struct nesdis_pdb *npdb);
static int verify_wmo_header(char *header);
static void output(char *fname,
		   struct nesdis_pdb *npdb,
		   int option_extended_info);
static char *make_default_pngname(struct nesdis_pdb *npdb);

int main(int argc, char **argv){

  int status = 0;
  int c;
  char *in_file;
  char *optstr = "hCbd:egino:p:s";
  char *usage = "nbspsat [-C|-i|-g|-n] [-b] [-o name] [-d outputdir] [-e]"
    " [-p prefix] [-s] file";
  int conflict_Cign = 0;
  int conflict_is = 0;
  int conflict_op = 0;

  set_progname(basename(argv[0]));

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

  if(optind != argc - 1)
    log_errx(1, "Needs one argument.");

  in_file = argv[optind];

  if(g.opt_background == 1)
    set_usesyslog();

  if(status == 0)
    status = verify_wmo_header(in_file);

  if(status == 0){
    if(g.opt_wrinfo == 1)
      status = write_file_info(in_file);
    else
      status = process_file(in_file);
  }

  return(status != 0 ? 1 : 0);
}

static int process_file(char *in_file){

  struct nesdis_pdb npdb;
  unsigned char *data = NULL;	/* records data (after pdb) */
  size_t data_size;		/* linesize * numlines */
  int n;
  int fd;
  int gempak_fd;
  FILE *fp;
  int status = 0;
  int zstatus = 0;
  char *fname;

  /* Initialize */
  npdb.buffer_size = NESDIS_WMO_HEADER_SIZE + NESDIS_PDB_SIZE;

  fd = open(in_file, O_RDONLY);
  if(fd == -1)
    log_err(1, "Could not open %s", in_file);

  if(read_nesdis_pdb(fd, &npdb) == -1)
    log_err(1, "Error from read_nesdis_pdb(): %s", in_file);

  if(g.opt_C == 1){
    close(fd);
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

    n = read(fd, data, data_size);
    if(n == -1)
      log_err(1, "Error reading from %s", in_file);
    else if((size_t)n != data_size)
      log_errx(1, "File is corrupt (short): %s", in_file);
  }

  if(g.opt_output_dir != NULL){
    status = chdir(g.opt_output_dir);
    if(status != 0)
      log_err(1, "Cannot chdir to %s", g.opt_output_dir);
  }

  if(g.opt_output_fname == NULL){
    fname = make_default_pngname(&npdb);
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

  if(g.opt_gempak_output){
    fname[strlen(fname) - 4] = '\0';
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
  close(fd);

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
			struct nesdis_pdb *npdb, int *zstatus){
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

static int write_file_info(char *in_file){
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
  struct nesdis_pdb npdb;
  char *output_fname;
  int status = 0;

  if(in_file != NULL){
    fd = open(in_file, O_RDONLY);
    if(fd == -1){
      log_err(1, "Could not open %s", in_file);
      return(-1);
    }
  }else
    fd = fileno(stdin);

  status = read_nesdis_pdb_compressed(fd, &npdb);
  if(in_file != NULL){
    close(fd);
  }

  if(status == -1){
    if(in_file != NULL)
      log_err(1, "Error reading from %s", in_file);
    else
      log_err(1, "Error reading from stdin.");

    return(-1);
  }else{
    if(status == 1)
      log_errx(1, "Error reading pdb. File too short.");
    else
      log_errx(1, "Could not read nesdis pdb. Error from zlib.");

    return(1);
  }

  output_fname = make_default_pngname(&npdb);
  if(output_fname == NULL)
    log_err(1, "Error from malloc().");

  output(output_fname, &npdb, g.opt_extended_info);

  return(0);
}

static void check(struct nesdis_pdb *npdb){
  
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
		   struct nesdis_pdb *npdb,
		   int option_extended_info){

  char time[TIME_STR_SIZE + 1];

  sprintf(time, "%d%02d%02d_%02d%02d",
	    npdb->year, npdb->month, npdb->day, npdb->hour, npdb->min);

  fprintf(stdout, "%d %d %d %d %d %s %s",
	  npdb->source, 
	  npdb->creating_entity, 
	  npdb->sector, 
	  npdb->channel,
	  npdb->res,
	  time,
	  fname);

  if(option_extended_info == 0){
    fprintf(stdout, "\n");
    return;
  }

  fprintf(stdout, " %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
	  npdb->map_projection,
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

static int verify_wmo_header(char *in_file){
  /*
   * Check the first NESDIS_WMO_HEADER_SIZE bytes to see if it consists of
   * 18 ascii bytes plus the \r\r\n.
   */
  int i;
  char *c;
  char header[NESDIS_WMO_HEADER_SIZE];
  int fd;
  int n;

  fd = open(in_file, O_RDONLY);
  if(fd == -1){
    log_err(1, "Could not open %s", in_file);
    return(-1);
  }

  n = read(fd, header, NESDIS_WMO_HEADER_SIZE);
  close(fd);

  if(n < NESDIS_WMO_HEADER_SIZE)
    goto end;

  c = header;
  for(i = 1; i <= NESDIS_WMO_HEADER_SIZE - 3; ++i){    
    if(isascii(*c++) == 0)
      goto end;
  }

  if((*c++ == '\r') && (*c++ == '\r') && (*c == '\n'))
    return(0);

 end:
  log_errx(1, "Incorrect header in file.");

  return(1);
}

static char *make_default_pngname(struct nesdis_pdb *npdb){

  char *fname;
  char prefix[NESDIS_WMOID_SIZE + 1];

  fname = malloc(PNG_FNAME_SIZE + 1);
  if(fname == NULL)
    return(NULL);

  if(g.opt_fname_prefix != NULL)
    strncpy(prefix, g.opt_fname_prefix, NESDIS_WMOID_SIZE);
  else
    strncpy(prefix, npdb->wmoid, NESDIS_WMOID_SIZE);

  prefix[NESDIS_WMOID_SIZE] = '\0';

  sprintf(fname, NBSPSAT_DEFAULT_PNGNAME_FMT, prefix, 
	  npdb->year, npdb->month, npdb->day, npdb->hour, npdb->min);

  return(fname);
}

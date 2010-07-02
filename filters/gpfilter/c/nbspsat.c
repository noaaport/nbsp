/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
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
#include "unz.h"
#include "const.h"

/*
 * This program assumes that the input file is the uncompressed gini data
 * file. Only when it is invoked with [-i] to just extract the relevant info,
 * it can take either the compressed or uncompresed file as input.
 */

#define WMO_HEADER_SIZE		CTRLHDR_WMO_SIZE	/* common.h */
#define WMOID_SIZE		WMO_ID_SIZE		/* common.h */
#define NESDIS_PDB_SIZE		512
#define NESDIS_DATA_BLOCKSIZE	5120
#define GEMPAK_FNAME_SIZE	20	  /* e.g. tigp04_yyyymmdd_hhmm */
#define PNG_FNAME_SIZE		24	  /* e.g. tigp04_yyyymmdd_hhmm.png */
#define TIME_STR_SIZE		13	  /* yyyymmdd_hhmm */

struct nesdis_product_def_block {
  char buffer[WMO_HEADER_SIZE + NESDIS_PDB_SIZE];
  char wmoid[WMOID_SIZE + 1];
  int  buffer_size;
  /* From AAO13008.pdf (p. 26) */
  int source;			/* should be 1 => NESDIS */
  int creating_entity;
  int sector;
  int channel;
  int numlines;
  int linesize;
  int year;
  int month;
  int day;
  int hour;
  int min;
  int secs;
  int hsecs;	/* hundredths of second */
  int nx;
  int ny;
  int res;	/* resolution */
};

struct {
  char *opt_output_dir;
  char *opt_fname_prefix;
  char *opt_output_fname;
  int opt_C;
  int opt_silent;
  int opt_gempak_output;
  int opt_png_output;
  int opt_background;
  int opt_wrinfo;	/* only extract and print info [-i] */
} g = {NULL, NULL, NULL, 0, 0, 0, 0, 0, 0};

static int read_nesdis_pdb(int fd, struct nesdis_product_def_block *npdb);
static void fill_nesdis_pdb(struct nesdis_product_def_block *npdb);
static int write_pngdata(FILE *fp, int fd, int linesize, int numlines);
static int write_gempak(int gempak_fd, int fd,
       struct nesdis_product_def_block *npdb, int *unz_status);
static int process_file(char *in_file);
static int write_file_info(char *in_file);
static void check(struct nesdis_product_def_block *npdb);
static int verify_wmo_header(char *header);
static void output(char *fname, struct nesdis_product_def_block *npdb);
static char *make_default_pngname(struct nesdis_product_def_block *npdb);

int main(int argc, char **argv){

  int status = 0;
  int c;
  char *in_file;
  char *optstr = "hCbd:gino:p:s";
  char *usage = "nbspsat [-C] [-b] [[-o name] | [-d outputdir]"
    " [-p prefix]] [-g] [-n] [-s | -i] [-C] file";
  int conflict = 0;

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'C':
      g.opt_C = 1;
      break;
    case 'b':
      g.opt_background = 1;
      break;
    case 'd':
      g.opt_output_dir = optarg;
      break;
    case 'g':
      g.opt_gempak_output = 1;
      break;
    case 'n':
      g.opt_png_output = 1;
      break;
    case 'i':
      if(g.opt_silent == 1)
	conflict = 1;

      g.opt_wrinfo = 1;
      break;
    case 'o':
      if(g.opt_fname_prefix != NULL)
	conflict = 1;

      g.opt_output_fname = optarg;
      break;
    case 'p':
      if(g.opt_output_fname != NULL)
	conflict = 1;

      g.opt_fname_prefix = optarg;
      break;
    case 's':
      if(g.opt_wrinfo == 1)
	conflict = 1;

      g.opt_silent = 1;
      break;
    case 'h':
    default:
      log_info(usage);
      exit(0);
      break;
    }

    if(conflict != 0){
      log_errx(1, "Conflicting options. %s", usage);
    }
  }

  if(optind != argc - 1)
    log_errx(1, "Needs one argument.");

  in_file = argv[optind];

  if(g.opt_background == 1)
    set_usesyslog();

  if((g.opt_gempak_output == 0) && (g.opt_png_output == 0)){
    g.opt_png_output = 1;
  }

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

  int fd;
  int gempak_fd;
  FILE *fp;
  struct nesdis_product_def_block npdb;
  int status = 0;
  int zstatus = 0;
  char *fname;

  /* Initialize */
  npdb.buffer_size = WMO_HEADER_SIZE + NESDIS_PDB_SIZE;

  fd = open(in_file, O_RDONLY);
  if(fd == -1)
    log_err(1, "Could not open %s", in_file);

  if(read_nesdis_pdb(fd, &npdb) == -1)
    log_err(1, "read_nesdis_pdb()");

  if(g.opt_C == 1){
    close(fd);
    check(&npdb);
    return(0);
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
    fname = malloc(strlen(g.opt_output_fname) + 5); /* ".png" plus \0 */
    if(fname == NULL)
      log_err(1, "malloc");

    sprintf(fname, "%s.png", g.opt_output_fname);
  }

  if(g.opt_png_output == 1){
    fp = fopen_output(fname, "w");
    if(fp == NULL)
      log_err(1, "Could not open %s", fname);
    
    if((status = write_pngdata(fp, fd, npdb.linesize, npdb.numlines + 1))
       != 0){
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

    status = write_gempak(gempak_fd, fd, &npdb, &zstatus);
    if(status == -1)
      log_err(1, "Could not write to %s", fname);
    else if(status == 1)
      log_errx(1, "zip error %d compressing %s", zstatus, fname);

    close(gempak_fd);
  }

  if(g.opt_silent == 0)
    output(fname, &npdb);

  free(fname);
  close(fd);

  return(0);
}

static int read_nesdis_pdb(int fd, struct nesdis_product_def_block *npdb){

  int n;

  n = read(fd, npdb->buffer, npdb->buffer_size);
  if(n == -1)
    return(-1);
  else if(n != npdb->buffer_size)
    return(1);

  fill_nesdis_pdb(npdb);

  return(0);
}

static void fill_nesdis_pdb(struct nesdis_product_def_block *npdb){

  unsigned char *p;
  int n;

  p = (unsigned char*)npdb->buffer;
  p += WMO_HEADER_SIZE;

  npdb->source = (int)p[0];
  npdb->creating_entity = (int)p[1];
  npdb->sector = (int)p[2];
  npdb->channel = (int)p[3];

  npdb->numlines = (int)((p[4] << 8) + p[5]);
  npdb->linesize = (int)((p[6] << 8) + p[7]);

  npdb->year = (int)p[8];
  npdb->year += 1900;
  npdb->month = (int)p[9];
  npdb->day = (int)p[10];
  npdb->hour = (int)p[11];
  npdb->min = (int)p[12];
  npdb->secs = (int)p[13];
  npdb->hsecs = (int)p[14];

  npdb->nx = (int)((p[16] << 8) + p[17]);
  npdb->ny = (int)((p[18] << 8) + p[19]);

  npdb->res = (int)p[41];

  for(n = 0; n <= WMOID_SIZE - 1; ++n)
    npdb->wmoid[n] = tolower(npdb->buffer[n]);

  npdb->wmoid[WMOID_SIZE] = '\0';
}

static int write_pngdata(FILE *fp, int fd, int linesize, int numlines){
  
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
  int n;
  volatile int status = 0;   /*
			      * warning: variable 'status' might be
			      * clobbered by `longjmp'
			      */

  row = malloc(linesize);
  if(row == NULL)
    return(-1);

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

  for(i = 0; i <= numlines - 1; ++i){
    n = read(fd, row, linesize);
    if(n == -1){
      status = -1;
      break;
    }else if(n != linesize){
      status = -1;
      break;
    }
    png_write_row(png_ptr, row);
  }

  if(status == 0)
    png_write_end(png_ptr, info_ptr);

 end:
  
  if(row != NULL)
    free(row);

  if(png_ptr != NULL)
    png_destroy_write_struct(&png_ptr, &info_ptr);

  return(status);
}

static int write_gempak(int gempak_fd, int fd, 
		 struct nesdis_product_def_block *npdb, int *zstatus){
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

  if(write(gempak_fd, npdb->buffer, WMO_HEADER_SIZE) == -1)
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
    if(n == -1)
      status = -1;

    if(status == 0){
      *zstatus = zip(&zblock, &zblock_size, block, n, level);
      if(*zstatus == 0){
	if(write(gempak_fd, zblock, zblock_size) == -1)
	  status = -1;

	free(zblock);
      }else
	status = 1;
    }

    if(status != 0)
      break;
  }

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
  int n;
  int i;
  char *output_fname;
  struct nesdis_product_def_block npdb;
  char buffer[WMO_HEADER_SIZE + NESDIS_PDB_SIZE];
  int buffer_size = WMO_HEADER_SIZE + NESDIS_PDB_SIZE;
  unsigned char *p;
  int status = 0;

  /* Initialize */
  npdb.buffer_size = WMO_HEADER_SIZE + NESDIS_PDB_SIZE;

  fd = open(in_file, O_RDONLY);
  if(fd == -1){
    log_err(1, "Could not open %s", in_file);
    return(-1);
  }

  n = read(fd, buffer, buffer_size);
  close(fd);

  if(n == -1){
    log_err(1, "Error reading from %s", in_file);
    return(-1);
  }

  p = (unsigned char*)&buffer[WMO_HEADER_SIZE];
  if((p[0] == ZBYTE0) && (p[1] == ZBYTE1)){
    /*
     * Must decompress. Find the end of the compressed frame.
     */
    n -= WMO_HEADER_SIZE;	/* number of bytes in buffer after wmo */
    i = 0;
    status = Z_DATA_ERROR;
    while(i <= n - 1){
      ++i;
      if((p[i] == ZBYTE0) && (p[i + 1] == ZBYTE1)){
	/*
	 * This should be the start of the next frame. Try to decompress
	 * what we have. If we get an error, try appending more data
	 * until the next marker and keep trying until we get to the end
	 * of the available data.
	 */
	status = unz(npdb.buffer, &npdb.buffer_size, (char*)p, i);
	if(status == Z_OK)
	  break;
      }
    }

    if(status != Z_OK){
      log_errx(1, "Could not read nesdis pdb from %s.", in_file);
      return(1);
    }
  }else{
    /*
     * The frame is uncmpressed.
     */
    if(n != npdb.buffer_size){
      log_errx(1, "Error reading from %s. File too short.", in_file);
      return(-1);
    }else
      memcpy(npdb.buffer, buffer, n);
  }

  fill_nesdis_pdb(&npdb);
  output_fname = make_default_pngname(&npdb);
  if(output_fname == NULL)
    log_err(1, "malloc()");

  output(output_fname, &npdb);

  return(0);
}

static void check(struct nesdis_product_def_block *npdb){
  
  fprintf(stdout, "%d%02d%02d_%02d%02d\n", 
	  npdb->year, npdb->month, npdb->day, npdb->hour, npdb->min);

  fprintf(stdout, "%d %d %d\n", npdb->numlines, npdb->linesize,
	  (npdb->numlines + 1) * npdb->linesize + WMO_HEADER_SIZE + 
	  NESDIS_PDB_SIZE);

  fprintf(stdout, "nx, ny: %d %d\n", npdb->nx, npdb->ny);

  fprintf(stdout, " source: %d\n", npdb->source);
  fprintf(stdout, " entity: %d\n", npdb->creating_entity);
  fprintf(stdout, " sector: %d\n", npdb->sector);
  fprintf(stdout, "channel: %d\n", npdb->channel);
  fprintf(stdout, "    res: %d\n", npdb->res);
}

static void output(char *fname, struct nesdis_product_def_block *npdb){

  char time[TIME_STR_SIZE + 1];

  sprintf(time, "%d%02d%02d_%02d%02d",
	    npdb->year, npdb->month, npdb->day, npdb->hour, npdb->min);

  fprintf(stdout, "%d %d %d %d %d %s %s\n", 
	  npdb->source, 
	  npdb->creating_entity, 
	  npdb->sector, 
	  npdb->channel,
	  npdb->res,
	  time,
	  fname);
}

static int verify_wmo_header(char *in_file){
  /*
   * Check the first WMO_HEADER_SIZE bytes to see if it consists of
   * 18 ascii bytes plus the \r\r\n.
   */
  int i;
  char *c;
  char header[WMO_HEADER_SIZE];
  int fd;
  int n;

  fd = open(in_file, O_RDONLY);
  if(fd == -1){
    log_err(1, "Could not open %s", in_file);
    return(-1);
  }

  n = read(fd, header, WMO_HEADER_SIZE);
  close(fd);

  if(n < WMO_HEADER_SIZE)
    goto end;

  c = header;
  for(i = 1; i <= WMO_HEADER_SIZE - 3; ++i){    
    if(isascii(*c++) == 0)
      goto end;
  }

  if((*c++ == '\r') && (*c++ == '\r') && (*c == '\n'))
    return(0);

 end:
  log_errx(1, "Incorrect header in file.");

  return(1);
}

static char *make_default_pngname(struct nesdis_product_def_block *npdb){

  char *fname;
  char prefix[WMOID_SIZE + 1];

  fname = malloc(PNG_FNAME_SIZE + 1);
  if(fname == NULL)
    return(NULL);

  if(g.opt_fname_prefix != NULL)
    strncpy(prefix, g.opt_fname_prefix, WMOID_SIZE);
  else
    strncpy(prefix, npdb->wmoid, WMOID_SIZE);

  prefix[WMOID_SIZE] = '\0';

  sprintf(fname, "%s_%d%02d%02d_%02d%02d.png", prefix, 
	  npdb->year, npdb->month, npdb->day, npdb->hour, npdb->min);

  return(fname);
}

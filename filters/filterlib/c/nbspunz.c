/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

/*
 * nbspunz [-b] [[-c skipcount] | -C] [-n nframes] [-o output] fname
 *
 * -c => cut <count> bytes from the output
 * -C => cut ccb header (24 bytes) from the output
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <zlib.h>
#include "util.h"
#include "err.h"
#include "mfile.h"
#include "misc.h"
#include "const.h"

#define MAX_FRAME_SIZE	MAX_FRDATA_SIZE
#define MEMFILE_BLOCKSIZE 2048

#define STATIC_BUFFER_SIZE	65536
static char gstatic_buffer[STATIC_BUFFER_SIZE];
static char *gmalloc_buffer = NULL;

static int filesize(char *fname);
static char *fill_buffer(char *fname, int *size);
static unsigned char *skip_header(int *buffer_size);
static int process_frames(struct memfile_st *mf,
			  unsigned char *buffer, int buffersize);
static int process_frame(struct memfile_st *mf, z_stream *zs, int *zstatus);
static int parse_args(int argc, char ** argv);
static int validate_args(void);

static unsigned char *gbuffer = NULL;
static struct memfile_st *gmf = NULL;
static int gfd = -1;

static char *ginputfname = NULL;
static char *goutputfname = NULL;
static int opt_background = 0;
static int opt_skipcount = 0;	/* optionally strip ccb, wmo, ... or any */
static int opt_nframes = 0;     /* process only this number of frames */
static int gframenumber = 0;
static char *usage = "nbspunz [-b] [[-c skipcount] | -C] [-n nframes]"
                     " [-o output] fname";

static void cleanup(void){

  if((goutputfname != NULL) && (gfd != -1)){
    close(gfd);
    gfd = -1;
  }

  if(gmalloc_buffer != NULL){
    free(gmalloc_buffer);
    gmalloc_buffer = NULL;
  }

  if(gmf != NULL){
    close_memfile(gmf);
    gmf = NULL;
  }
}

static int filesize(char *fname){

  struct stat sb;

  if(stat(fname, &sb) == -1)
    return(-1);

  return(sb.st_size);
}

static char *fill_buffer(char *fname, int *buffer_size){

  int size;
  int fd;
  char *p;

  size = filesize(fname);
  if(size == -1)
    return(NULL);

  if(size <= STATIC_BUFFER_SIZE)
    p = gstatic_buffer;
  else{
    gmalloc_buffer = malloc(size);
    p = gmalloc_buffer;
  }

  if(p == NULL)
    return(NULL);

  fd = open(fname, O_RDONLY);
  if(fd == -1){
    return(NULL);
  }

  if(read(fd, p, size) != size){
    p = NULL;
  }else
    *buffer_size = size;

  close(fd);

  return(p);
}

static unsigned char *skip_header(int *buffer_size){

  unsigned char *p;
  unsigned char *r = NULL;
  int b;

  b = 0;
  p = gbuffer;
  while(b <= *buffer_size - 2){
    if((*p == ZBYTE0) && (*(p + 1) == ZBYTE1)){
      r = p;
      break;
    }

    ++p;
    ++b;
  }
  
  *buffer_size -= b;

  return(r);
}

static int process_frames(struct memfile_st *mf,
			  unsigned char *buffer, int buffer_size){
  /*
   * The strategy here is:  Initialize a zstream and begin to uncompress
   * the input buffer. When inflate() returns Z_STREAM_END, it means
   * it has reached the end of one frame. We then save the amount
   * that remains to be read from the input buffer, re-initialize the
   * zstream, and the restore the avail_in and next_in field so that
   * we can call inflate again to continue uncompressing with the next
   * frame, from where it ended. We continue with this until the
   * input buffer is consumed.
   */
  z_stream zs;
  int save_avail_in;
  int zstatus = 0;
  int status = 0;

  zs.zalloc = NULL;
  zs.zfree = NULL;
  zs.next_in = buffer;
  zs.avail_in = buffer_size;
  zstatus = inflateInit(&zs);
  if(zstatus != Z_OK){
    status = 2;
  }

  while((zs.avail_in > 0) && (status == 0)){
    status = process_frame(mf, &zs, &zstatus);
    ++gframenumber;
    if(gframenumber == opt_nframes)
      break;

    if((status == 0) && (zstatus == Z_STREAM_END)){
      save_avail_in = zs.avail_in;
      inflateEnd(&zs);
      zs.zalloc = NULL;
      zs.zfree = NULL;
      zs.avail_in = save_avail_in;
      zs.next_in = &buffer[buffer_size - save_avail_in];
      zstatus = inflateInit(&zs);
      if(zstatus != Z_OK){
	status = 2;
      }
    }
  }

  if(status != 2)
    inflateEnd(&zs);

  if(status == -1)
    log_err(1, "Error processing frame in %s", ginputfname);
  else if(status == 1)
    log_errx(1, "Error %d from inflate in %s.", zstatus, ginputfname);
  else if(status == 2)
    log_errx(1, "Error %d from inflateInit in %s.", zstatus, ginputfname); 

  return(status);
}

static int process_frame(struct memfile_st *mf, z_stream *zs, int *zstatus){
  /*
   * Sometimes the last frame is transmitted uncompressed. When that is the
   * case, inflate returns Z_DATA_ERROR. We could check for that error
   * and assume that it is for that reason, or check explicitly ourselves
   * whether or not the frame is compressed by looking at the first two
   * bytes.
   * 
   * Returns:
   *
   *  0
   *  1 => error from inflate (zstatus has the error code)
   * -1 => mf write error
   */
  unsigned char outbuf[MAX_FRAME_SIZE];
  int outlen = MAX_FRAME_SIZE;
  int status = 0;
  int n;
  int skip = 0;		
  int write_out;

  /* 
   * The default is write the entire frame, but the first frame is stripped
   * the number of bytes specifies by the [-c] option.
   */
  if((gframenumber == 0) && (opt_skipcount > 0))
    skip = opt_skipcount;

  if((zs->next_in[0] != ZBYTE0) || (zs->next_in[1] != ZBYTE1)){
    if((n = write_memfile(mf, zs->next_in, zs->avail_in)) == -1)
      status = -1;
    else
      zs->avail_in = 0;

    return(status);
  }

  zs->next_out = outbuf;
  zs->avail_out = outlen;    
  *zstatus = inflate(zs, Z_SYNC_FLUSH);

  if((*zstatus == Z_OK) || (*zstatus == Z_STREAM_END)){
    write_out = outlen - zs->avail_out;
    if(skip >= write_out)
      log_errx(1, "Skip count too large.");
    else
      write_out -= skip;

    if((n = write_memfile(mf, outbuf + skip, write_out)) == -1)
      status = -1;
  }else 
    status = 1;

  return(status);
}

int main(int argc, char **argv){

  int status = 0;
  int buffer_size = -1;	/* just to ensure it is initalized below */
  unsigned char *zbuffer;

  set_progname(basename(argv[0]));

  status = parse_args(argc, argv);
  if(status == 0)
    status = validate_args();

  if(status != 0)
    exit(EXIT_FAILURE);

  if(opt_background == 1)
    set_usesyslog();

  atexit(cleanup);

  gbuffer = (unsigned char*)fill_buffer(ginputfname, &buffer_size);
  if(gbuffer == NULL)
    log_err(1, "fill_buffer");

  if(goutputfname != NULL){
    gfd = open(goutputfname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(gfd == -1)
      log_err(1, "open");
  }else
    gfd = fileno(stdout);

  gmf = open_memfile(1, MEMFILE_BLOCKSIZE);
  if(gmf == NULL)
    log_err(1, "open_memfile");

  zbuffer = (unsigned char*)skip_header(&buffer_size);
  if(zbuffer == NULL)
    log_errx(1, "No compressed frames found in %s.", ginputfname);

  status = process_frames(gmf, zbuffer, buffer_size);
  if(status != 0)
    log_errx(1, "Error uncompressing %s.", ginputfname);

  if(save_memfile(gfd, gmf) == -1)
    log_err(1, "Error processing %s.", ginputfname);

  return(status);
}

static int parse_args(int argc, char ** argv){

  char *optstr = "bCc:n:o:";
  int status = 0;
  int c;
  int opt_cC = 0;	/* to check for -c and -C conflict */

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      opt_background = 1;
      break;
    case 'c':
      ++opt_cC;
      status = strto_int(optarg, &opt_skipcount);
      if(status == 1){
	log_errx(1, "Invalid argument to [-c] option.");
      }
      break;
    case 'C':
      ++opt_cC;
      opt_skipcount = CCB_SIZE;
      break;
    case 'n':
      status = strto_int(optarg, &opt_nframes);
      if(status == 1){
	log_errx(1, "Invalid argument to [-n] option.");
      }
      break;
    case 'o':
      goutputfname = optarg;
      break;
    default:
      status = 1;
      log_errx(1, usage);
      break;
    }
  }

  if(opt_cC > 1)
    log_errx(1, "Conflicting options -c and -C");

  if(optind <= argc - 1)
    ginputfname = argv[optind];

  return(status);
}

static int validate_args(void){

  int status = 0;

  if(ginputfname == NULL){
    status = 1;
    log_errx(1, usage);
  }

  if(opt_skipcount < 0){
    status = 1;
    log_errx(1, "Illegal value of [-c] option.");
  }

  return(status);
}

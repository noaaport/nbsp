/*
 * Copyright (c) 2009 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

/*
 * Reader of the nbsp cspool.
 *
 * Usage: nbspcspoolr [-v] [-b] [-q] [-d <dbhome>] [-f <dbfname>]
 *	              [-c <cache_mb>] [-m] [-n <Ndb>] [-s <pagesize>]
 *		      [<key>,<size> <key>,<size> ...]
 *
 * The [-m] option sets the f_mpool_nofile flag in the cspoolbdb_create()
 * to open a shared memory bdb. (Otherwise it is a normal file-backed bdb).
 *
 * Writes the contents of the file to stdout.
  * If no argument is given, it takes them from stdin, each line of the form
 *
 * <key>,<size>
 *
 * If size is 0, then the complete file data is returned.
 *
 * Output:  Without the [-q] option the output is
 *
 *          <code><size><data>
 *
 * and with [-q] the <code> and <size> are not output.
 *
 * <code> is a three-character string:
 *
 * 000  =>  no errors
 * 001  =>  key not found
 * 255  =>  error
 *
 * and <size> is an 8-character string giving the size of the data as a
 * hexadecimal string (without the leading 0x). When there is an error
 * <data> is an error message.
 * 
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include "util.h"
#include "err.h"
#include "mspoolbdb.h"

#define CSPOOL_CODE_SIZE  3
#define CSPOOL_CODE_OK "000"
#define CSPOOL_CODE_NOTFOUND "001"
#define CSPOOL_CODE_ERROR "255"
#define HEADER_STR_LEN 11	            /* use this + 1 in snprintf */
#define HEADER_STR_FMT "%.3s%08x"           /* for the <code><size> */
#define CSPOOL_KEY_SIZE_SEP_CHAR ','	    /* <key>,<size> in input */

#define STATIC_PAGE_BUFFER_SIZE 131072	/* 128 KB */
#define MIN_STATIC_PAGE_BUFFER_SIZE 4096
static char static_page_buffer[STATIC_PAGE_BUFFER_SIZE];

#define STATIC_INPUT_LINE_SIZE 4096
static char static_input_line[STATIC_INPUT_LINE_SIZE];

static struct {
  int opt_verbose;
  int opt_background;
  int opt_q;		   /* no <code><size> codes */
  size_t opt_page_size;    /* requested page size */
  int opt_mpool_nofile;    /* [-m] */
  /* variables */
  struct mspoolbdb_st *mspool;
  char *dbhome;
  char *dbfname;
  uint32_t dbcache_mb;
  unsigned int Ndb;
  char *key;
  size_t requested_size;
  int sd;
  char *page;
  size_t page_size;
  char *malloc_buffer;
} g = {0, 0, 0, 0, 0, NULL, NULL, NULL, 0, 0, NULL, 0, -1,
       static_page_buffer, STATIC_PAGE_BUFFER_SIZE, NULL};

static int spool_open(void);
static int spool_close(void);
static int read_file(void);
static void cleanup(void);
static int process_argv(int argc, char **argv);
static int process_stdin(void);
static ssize_t nwrite(int fd, const void *buf, size_t nbytes);

int main(int argc, char **argv){

  char *optstr = "hbqvd:f:c:mn:s:";
  char *usage = "nbspcspoolr [-v] [-b] [-d <dbhome>] [-f <dbfname>] "
    "[-c <cache_mb>] [-n <Ndb>] [-s <buffersize>] <key>";
  int c;
  int status = 0;
  uint32_t buffersize;

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'q':
      g.opt_q = 1;
      break;
    case 'v':
      ++g.opt_verbose;
      break;
    case 'd':
      g.dbhome = optarg;
      break;
    case 'f':
      g.dbfname = optarg;
      break;
    case 'c':
      status = strto_u32(optarg, &g.dbcache_mb);
      if(status != 0){
        log_errx(1, "Invalid value for [-c].");
      }
      break;
    case 'm':
      g.opt_mpool_nofile = 1;
      break;
    case 'n':
      status = strto_uint(optarg, &g.Ndb);
      if(status != 0){
        log_errx(1, "Invalid value for [-n].");
      }
      break;
    case 's':
      status = strto_u32(optarg, &buffersize);
      if(status != 0){
        log_errx(1, "Invalid value for [-s].");
      }
      if(buffersize < MIN_STATIC_PAGE_BUFFER_SIZE)
	buffersize = MIN_STATIC_PAGE_BUFFER_SIZE;

      g.opt_page_size = (size_t)buffersize;
      break;
    case 'h':
    default:
      log_errx(1, usage);
      break;
    }
  }
  
  if((g.dbhome == NULL) || (g.dbfname == NULL) || (g.dbcache_mb == 0) ||
     (g.Ndb == 0)){
    log_errx(1, usage);
  }

  if(g.opt_page_size != g.page_size){
    g.malloc_buffer = malloc(g.opt_page_size);
    if(g.malloc_buffer == NULL)
      log_err(1, "Cannot initialize.");

    g.page = g.malloc_buffer;
    g.page_size = g.opt_page_size;
  }

  atexit(cleanup);

  if(g.opt_background == 1)
    set_usesyslog();

  if(optind < argc){
    argc -= optind;
    argv += optind;
    status = process_argv(argc, argv);
  }else{
    status = process_stdin();
  }

  return(status);
}

static int spool_open(void){

  int status;
  int dbfile_mode = 0; /* ignored */
  int dbenv_mode = 0;  /* ignored */
  unsigned int maxsize_per128 = 0; /* irrelevant */
  int nslots = 1;      /* only read one file at a time */	
  int f_rdonly = 1;

  status = cspoolbdb_create(&g.mspool,
			    g.dbhome, dbenv_mode,
			    g.dbcache_mb, maxsize_per128,
			    g.dbfname, dbfile_mode,
			    g.Ndb, nslots,
			    g.opt_mpool_nofile, f_rdonly);
  return(status);
}

static int spool_close(void){

  int status;

  if(g.mspool == NULL)
    return(0);

  status = cspoolbdb_destroy(g.mspool);
  g.mspool = NULL;

  return(status);
}

static void cleanup(void){

  if(g.malloc_buffer != NULL){
    free(g.malloc_buffer);
    g.malloc_buffer = NULL;
  }

  if(g.mspool != NULL){
    if(g.sd != -1){
      (void)cspoolbdb_slots_close(g.mspool, g.sd);
      g.sd = -1;
    }
    (void)spool_close();
  }
}

static int process_argv(int argc, char **argv){

  char *line = static_input_line;
  int line_max_size = STATIC_INPUT_LINE_SIZE;
  int status = 0;
  int i;
  char *size_str;
  uint32_t size;

  status = spool_open();
  if(status != 0){
    log_errx(1, "spool_open(): %s", db_strerror(status));
    return(-1);
  }

  for(i = 0; i < argc; ++i){
    g.key = argv[i];
    if(strlen(g.key) >= STATIC_INPUT_LINE_SIZE){
      log_warnx("Key %s exceeds maximum size %d.", g.key, line_max_size);
      continue;
    }
    strncpy(line, g.key, STATIC_INPUT_LINE_SIZE);
    size_str = strchr(line, CSPOOL_KEY_SIZE_SEP_CHAR);
    if(size_str == NULL)
      g.requested_size = 0;
    else{
      *size_str = '\0';
      ++size_str;
      status = strto_u32(size_str, &size);
      if(status != 0)
	log_errx(1, "Invalid <size> specified for key %s", g.key);
      
      g.requested_size = (size_t)size;
    }

    (void)read_file();
  }

  status = spool_close();

  if(status != 0)
    log_errx(1, "spool_close(): %s", db_strerror(status));
  else if(g.opt_verbose)
    log_info("Closed bdb memory spool.");

  return(0);
}

static int process_stdin(void){

  int status = 0;
  char *line = static_input_line;
  int line_max_size = STATIC_INPUT_LINE_SIZE;
  int line_size;
  int lineno = 0;
  char *size_str;
  uint32_t size;

  status = spool_open();
  if(status != 0)
    log_errx(1, "spool_open(): %s", db_strerror(status));

  while(fgets(line, line_max_size, stdin) != NULL){
    ++lineno;

    /* An empty line (just the '\n') is a signal to quit */
    if(line[0] == '\n')
      break;

    line_size = strlen(line);
    if(line[line_size - 1] != '\n'){
      log_warnx("Line %d exceeds maximum size %d.", lineno, line_max_size);
      continue;
    }
    line[line_size - 1] = '\0';
    
    g.key = line;

    size_str = strchr(g.key, CSPOOL_KEY_SIZE_SEP_CHAR);
    if(size_str == NULL)
      g.requested_size = 0;
    else{
      *size_str = '\0';
      ++size_str;
      status = strto_u32(size_str, &size);
      if(status != 0)
	log_errx(1, "Invalid <size> specified for key %s", g.key);
      
      g.requested_size = (size_t)size;
    }

    (void)read_file();
  }

  status = spool_close();

  if(status != 0)
    log_errx(1, "spool_close(): %s", db_strerror(status));
  else if(g.opt_verbose)
    log_info("Closed bdb memory spool.");

  return(0);
}

static ssize_t nwrite(int fd, const void *buffer, size_t n){

  size_t nleft;
  ssize_t nwritten;
  const char *p;

  p = (char*)buffer;
  nleft = n;
  while(nleft > 0) {
    nwritten = write(fd, p, nleft);
    if(nwritten < 0){
      log_warnx("nwrite(): %s", strerror(errno));
      return(-1);
    }

    nleft -= nwritten;
    p += nwritten;
  }

  return(n);
}

static int read_file(void){
  /*
   * Reads a file from the spool cache with the key and size given
   * in g.key and g.requested_size.
   */
  int status = 0;
  int output_status = 0;
  size_t n;
  size_t nleft;
  size_t file_size; /* we assume below that it fits in a uint32_t */
  char header[HEADER_STR_LEN + 1];
  char *code;

  status = cspoolbdb_slots_open_unlocked(g.mspool, g.key, &g.sd);
  if(status == 0){
    code = CSPOOL_CODE_OK;
    file_size = cspoolbdb_slots_datasize(g.mspool, g.sd);
    assert(file_size < 0xffffffffU);
    if((g.requested_size == 0) || (g.requested_size > file_size))
      g.requested_size = file_size;
  }else{
    g.requested_size = 0;
    if((status == DB_NOTFOUND) || (status == DB_PAGE_NOTFOUND)){
      code = CSPOOL_CODE_NOTFOUND;
      if(g.opt_verbose)
	log_warnx("cspoolbdb_slots_open(). key %s not found.", g.key);
    }else{
      log_warnx("cspoolbdb_slots_open(). key %s: %s",
		g.key, db_strerror(status));
      code = CSPOOL_CODE_ERROR;
    }
  }

  snprintf(header, HEADER_STR_LEN + 1, HEADER_STR_FMT,
	   code, (unsigned int)g.requested_size);

  if(g.opt_q == 0){
    if(nwrite(STDOUT_FILENO, header, HEADER_STR_LEN) == -1){
      status = -1;
      output_status = 1;
      goto End;
    }
  }
  
  if(status != 0)
    return(-1);

  if(g.opt_verbose > 1)
    log_info("%s found in spool cache", g.key);

  nleft = g.requested_size;
  while(nleft > 0){
    if(nleft > g.page_size)
      n = g.page_size;
    else
      n = nleft;

    n = cspoolbdb_slots_read(g.mspool, g.sd, (void*)g.page, n);
    if(n > 0){
      if(nwrite(STDOUT_FILENO, g.page, n) == -1){
	status = -1;
	output_status = 2;

	goto End;
      }
      nleft -= n;
    }
  }

  End:

  if(status != 0){
      log_warnx("read_file(): Error %d writing output.", output_status);
  }

  status = cspoolbdb_slots_close_unlocked(g.mspool, g.sd);
  g.sd = -1;

  if(status != 0)
    log_errx(1, "cspoolbdb_slots_close(). %s", db_strerror(status));

  return(status);
}

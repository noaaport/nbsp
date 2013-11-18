/*
 * Copyright (c) 2007-2013 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See the LICENSE file in the nbsp distribution.
 *
 * $Id: nbsp2ldm.c,v 4a7324491ec1 2013/11/13 01:56:32 nieves $
 *
 * Usage:
 *
 *   nbsp2ldm [OPTIONS] <filename>
 *   nbsp2ldm [OPTIONS] < stdin
 *   nbsp2ldm -S <filesize> [OPTIONS] < stdin
 *
 *   OPTIONS = [-b] [-c ccbsize] [-f feedtype] [-g] [-m] [-n] \
 *             [-o origin] [-p prodid] [-q pqfname] [-s seq]
 *
 *   In the first form the file name (path) is given in the argument.
 *   In the second form the list of file names is read from stdin. Each
 *   input line must be in the format given in the first line of the Usage.
 *   In the third form the file contents are read from stdin, and
 *    "-p prodid" is mandatory.
 */
#include <libgen.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/md5.h>		/* to avoid the ldm <md5.h> */
#include "ldm.h"
#include "pq.h"
#include "strsplit.h"
#include "err.h"

#define DEFAULT_PQFNAME		"/var/ldm/ldm.pq"
#define DEFAULT_PRODORIGIN	"nbsp"

#define DEFAULT_SEQNUM          90000
#define DEFAULT_FEEDTYPE	WMO
#define DEFAULT_CCB_SIZE	24

#define DEFAULT_STRSPLIT_DELIM ":,"

struct {
  int opt_background;           /* [-b] => use syslog */
  unsigned int opt_ccbsize;	/* [-c] */
  char *opt_strsplit_delim;     /* [-d] */
  unsigned int opt_feedtype;	/* [-f] */
  int opt_gempak;	/* [-g] add gempak trailer/footer */
  int opt_md5seq;	/* [-m] => calculate md5 from the [-s] seqnum str */
  int opt_noccb;	/* [-n] no ccb in file */
  char *opt_origin;	/* [-o] origin */
  char *opt_prodid;	/* [-p] */
  char *opt_pqfname;	/* [-q] */
  char *opt_seq_str;	/* [-s] => calculate md5 from the given sequence */
  unsigned int opt_filesize;	/* [-S] => must be given when file is stdin */
  /* variables */
  unsigned int seq;
  char *input_fname;
  struct pqueue *pq;
  struct strsplit_st *strp;
  char *dynamic_pool;
  int dynamic_pool_size;
  int fd;
} g = {0, DEFAULT_CCB_SIZE, DEFAULT_STRSPLIT_DELIM, DEFAULT_FEEDTYPE, 0, 0, 0,
       DEFAULT_PRODORIGIN, NULL,
       DEFAULT_PQFNAME, NULL,
       0, DEFAULT_SEQNUM, NULL, NULL, NULL, NULL, 0, -1};

struct {
  unsigned int opt_ccbsize;
  unsigned int opt_feedtype;
  int opt_gempak;
  int opt_md5seq;
  int opt_noccb;
  char *opt_origin;
  char *opt_pqfname;
} gdefault =  {DEFAULT_CCB_SIZE, DEFAULT_FEEDTYPE, 0, 0, 0,
	       DEFAULT_PRODORIGIN, DEFAULT_PQFNAME};

#define GMPK_TRAILER_SIZE	4
#define GMPK_HEADER_SIZE       11
char *gmpk_header_fmt = "\001\r\r\n%03d \r\r\n"; 
char *gmpk_trailer_str = "\r\r\n\003";
char gmpk_header_str[GMPK_HEADER_SIZE + 1];

#define STATIC_POOL_SIZE	1048576
char static_pool[STATIC_POOL_SIZE];

#define STDIN_CMDLINE_BUF_SIZE 1024
char stdin_cmdline_buf[STDIN_CMDLINE_BUF_SIZE];

static void init(void);
static void cleanup(void);
static void resetdefaults(void);
static void input_file_errx(int e, char *s);
static void input_file_err(int e, char *s);
static ssize_t readn(int fd, void *p, size_t n);
static int extract_uint(char *s, unsigned int *val);
static int calc_md5(unsigned char *buf, off_t bufsize, signaturet signature);
static void process_args(int argc, char **argv,
			 char *optstr, int f_set_defaults);

static int process_file(void);
static int process_stdin(void);
static int process_stdin_entry(void);

#ifdef TEST
static void test_print(void);
#endif

static void init(void){

  int status = 0;

#ifndef TEST
  status = pq_open(g.opt_pqfname, PQ_DEFAULT, &g.pq);
#endif

  if(status != 0){
    if(status == PQ_CORRUPT)
      log_errx(1, "The product queue %s is inconsistent.", g.opt_pqfname);
    else 
      log_err(1, "Error from pq_open()");
  }
}

static void cleanup(void){
  /*
   * If malloc() was used for prod.data, we leave it to the OS to free
   * the memory allocated (g.dynamic_pool).
   */
  if(g.pq != NULL) {
    pq_close(g.pq);
    g.pq = NULL;
  }  
  
  if(g.strp != NULL) {
    strsplit_delete(g.strp);
    g.strp = NULL;
  }

  if((g.fd != -1) && (g.fd != STDIN_FILENO)){
    close(g.fd);
    g.fd = -1;
  }
}

static void resetdefaults(void) {
  /*
   * Reset the default values after processing each file,
   * when processing more than one file.
   */

  g.opt_ccbsize = gdefault.opt_ccbsize;
  g.opt_feedtype = gdefault.opt_feedtype;
  g.opt_gempak = gdefault.opt_gempak;
  g.opt_md5seq = gdefault.opt_md5seq;
  g.opt_noccb = gdefault.opt_noccb;
  g.opt_origin = gdefault.opt_origin;
  g.opt_prodid = NULL;		/* there is no default for this */
  g.opt_pqfname = gdefault.opt_pqfname;
  g.opt_seq_str = NULL;	/* there is no default for this */
  g.input_fname = NULL;		/* there is no default for this */
}

static void input_file_errx(int e, char *s){

  if(g.input_fname == NULL)
      log_warnx("%s", s);
  else
      log_warnx("%s: %s", s, g.input_fname);

  if(e != 0)
    exit(e);
}

static void input_file_err(int e, char *s){

  if(g.input_fname == NULL)
    log_err(e, "%s", s);
  else
    log_err(e, "%s: %s", s, g.input_fname);
}

static ssize_t readn(int fd, void *p, size_t n){
  /*
   * Use when reading from the file contents from stdin.
   */
  size_t nleft;
  ssize_t nread;
  char *q;

  q = p;
  nleft = n;
  while(nleft > 0){
    if((nread = read(fd,q, nleft)) < 0)
      return(nread);
    else if(nread == 0)
      break;

    nleft -= nread;
    q += nread;
  }

  return(n - nleft);
}

static int extract_uint(char *s, unsigned int *val){

  char *end;
  int status = 0;
  unsigned int v;

  v = strtoul(s, &end, 10);
  if( (end == s) || (*end != '\0') )
    status = 1;
  
  if(status == 0)
    *val = v;

  return(status);
}

static int calc_md5(unsigned char *buf, off_t bufsize, signaturet signature){

  MD5_CTX md5;

  MD5_Init(&md5);
  MD5_Update(&md5, buf, bufsize);
  MD5_Final(signature, &md5);

  return(0);
}

static void process_args(int argc, char **argv,
			 char *optstr, int f_set_defaults) {

  char *usage = "nbsp2ldm [OPTIONS] <filename> \n\
nbsp2ldm [OPTIONS] < stdin \n\
nbsp2ldm -S <filesize> [OPTIONS] < stdin \n\
OPTIONS = [-b] [-c ccbsize] [-f feedtype] [-g] [-m] [-n] \n\
          [-o origin] [-p prodid] [-q pqfname] [-s seq]";
  int status = 0;
  int c;

  optind = 0;

  while((c = getopt(argc, argv, optstr)) != -1) {
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'c':
      status = extract_uint(optarg, &g.opt_ccbsize);
      if(status != 0)
	log_errx(1, "Invalid value for [-c].");

      if(f_set_defaults)
	gdefault.opt_ccbsize = g.opt_ccbsize;

      break;
    case 'd':
      g.opt_strsplit_delim = optarg;
      break;
    case 'f':
      status = extract_uint(optarg, &g.opt_feedtype);
      if(status != 0)
	log_errx(1, "Invalid value for [-f].");

      if(f_set_defaults)
	gdefault.opt_feedtype = g.opt_feedtype;

      break;
    case 'g':
      g.opt_gempak = 1;
      if(f_set_defaults)
	gdefault.opt_gempak = 1;

      break;
    case 'm':
      g.opt_md5seq = 1;
      if(f_set_defaults)
	gdefault.opt_md5seq = 1;

      break;
    case 'n':
      g.opt_noccb = 1;
      if(f_set_defaults)
	gdefault.opt_noccb = 1;

      break;
    case 'o':
      g.opt_origin = optarg;
      if(f_set_defaults)
	gdefault.opt_origin = optarg;

      break;
    case 'p':
      g.opt_prodid = optarg;
      break;
    case 'q':
      g.opt_pqfname = optarg;
      if(f_set_defaults)
	gdefault.opt_pqfname = optarg;

      break;
    case 's':
      g.opt_seq_str = optarg;
      status = extract_uint(optarg, &g.seq);
      if(status != 0)
	log_errx(1, "Invalid value for [-s].");

      break;
    case 'S':
      status = extract_uint(optarg, &g.opt_filesize);
      if(status != 0)
	log_errx(1, "Invalid value for [-S].");

      break;
    case 'h':
    default:
      status = 1;
      fprintf(stdout, "%s\n", usage);
      break;
    }
  }

  if(status != 0)
    exit(status);
}

int main(int argc, char **argv){
  
  char *optstr = "bc:d:f:ghmno:p:q:s:S:";
  int status = 0;

  set_progname(basename(argv[0]));

  process_args(argc, argv, optstr, 1);

  if(optind < argc - 1)
    log_errx(1, "Too many arguments.");
  else if(optind == argc - 1) {
    if(g.opt_filesize != 0) {
      log_errx(1, "Option [-S] is not valid when a file argument is given.");
    }
    g.input_fname = argv[optind++];
  } else if((g.opt_filesize != 0) && (g.opt_prodid == NULL))
      log_errx(1, "Option [-p] is mandatory if [-S] is given.");
  else if((g.opt_filesize == 0) && (g.opt_prodid != NULL))
      log_errx(1, "Option [-p] requires [-S] or a file argument");

  if(g.opt_background == 1)
    set_usesyslog();

  if(atexit(cleanup) != 0)
    log_err(1, "atexit");

  init();

  if(g.input_fname != NULL)
    status = process_file();
  else if(g.opt_filesize != 0)
    status = process_file();
  else
    status = process_stdin();

  return(status != 0 ? 1 : 0);
}

/*
 * File processing functions
 */

static int process_file(void) {
  /*
   * Process a file given in the command line or its data from stdin
   */
  struct stat statb;
  int ccb_offset = g.opt_ccbsize;
  int status = 0;
  product prod;
  ssize_t nread;
  ssize_t filedata_size;
  unsigned char *p;
  int n;

#ifdef TEST
  test_print();
  return(0);
#endif
  
  prod.info.origin = g.opt_origin;
  prod.info.feedtype = g.opt_feedtype;
  set_timestamp(&prod.info.arrival);
  prod.info.seqno = g.seq;
  prod.info.ident = g.opt_prodid;
  if(prod.info.ident == NULL)
    prod.info.ident = g.input_fname;

  /*
   * This should not happen because it has been checked in option processing.
   */
  if(prod.info.ident == NULL)
    log_errx(1, "No prodid specified.");

  if(g.input_fname == NULL){
    g.fd = STDIN_FILENO;
    filedata_size = g.opt_filesize;
  } else {
    g.fd = open(g.input_fname, O_RDONLY, 0);
    if(g.fd == -1)
      input_file_err(1, "Error from open()");
	
    if(g.opt_filesize == 0){
      if(fstat(g.fd, &statb) == -1)
	input_file_err(1, "Error from fstat()");
      
      filedata_size = statb.st_size;
    }else
      filedata_size = g.opt_filesize;
  }

  if(g.opt_noccb == 0){
    if(filedata_size <= g.opt_ccbsize)
      input_file_errx(1, "No data in inputfile");
    else
      filedata_size -= g.opt_ccbsize;
  }else
    ccb_offset = 0;

  prod.info.sz = filedata_size;
  if(g.opt_gempak == 1){
    prod.info.sz += GMPK_HEADER_SIZE + GMPK_TRAILER_SIZE;
  }

  if(prod.info.sz <= STATIC_POOL_SIZE)
    prod.data = (void*)static_pool;
  else if(prod.info.sz <= g.dynamic_pool_size)
    prod.data = g.dynamic_pool;
  else {
    prod.data = malloc(prod.info.sz);
    if(prod.data == NULL)
      log_err(1, "Error frrom malloc()");

    if(g.dynamic_pool != NULL)
      free(g.dynamic_pool);
    
    g.dynamic_pool = prod.data;
    g.dynamic_pool_size = prod.info.sz;
  }

  if(ccb_offset != 0){
    if(lseek(g.fd, ccb_offset, SEEK_SET) == -1)
      input_file_err(1, "Error from lseek()");
  }

  p = (unsigned char*)prod.data;

  if(g.opt_gempak == 1){
    n = sprintf(gmpk_header_str, gmpk_header_fmt, (int)(g.seq % 1000));
    if(n != GMPK_HEADER_SIZE)
      errx(1, "gmpk_header_fmt format error.");

    memcpy(p, gmpk_header_str, GMPK_HEADER_SIZE);
    p += GMPK_HEADER_SIZE;
  }

  /*
   * Use readn when nbsp2ldm is opened as a pipe in tcl.
   */
  nread = readn(g.fd, p, filedata_size);
  if(nread != filedata_size)
    input_file_err(1, "Error from read()");

  if((g.opt_md5seq == 1) && (g.opt_seq_str != NULL))
    status = calc_md5((unsigned char*)g.opt_seq_str,
		      strlen(g.opt_seq_str),
		      prod.info.signature);
  else
    status = calc_md5(p, filedata_size, prod.info.signature);

  if (status != 0)
    log_errx(1, "Error from calc_md5()");

  if(g.opt_gempak == 1){
    p += filedata_size;
    memcpy(p, gmpk_trailer_str, GMPK_TRAILER_SIZE);
  }

  status = pq_insert(g.pq, &prod);    
  if(status == PQUEUE_DUP){
    status = 0;
    log_warnx("Product already in queue: %s.", prod.info.ident);
  }else if(status != 0)
    log_errx(1, "Error from pq_insert: %d", status);

  if((g.fd != -1) && (g.fd != STDIN_FILENO)){
    close(g.fd);
    g.fd = -1;
  }

  resetdefaults();    /* reset the option parameters for the next file */
  
  return(status != 0 ? 1 : 0);
}

static int process_stdin(void) {

  char *s = &stdin_cmdline_buf[0];
  int size;
  int status = 0;

  while(fgets(s, STDIN_CMDLINE_BUF_SIZE, stdin) != NULL) {
    size = strlen(s);
    if(s[size - 1] != '\n') {
      status = 1;
      log_warnx("%s", "Line too long");
      break;
    }

    s[size - 1] = '\0';
    --size;

    g.strp = strsplit_recreate(s, g.opt_strsplit_delim,
			       STRSPLIT_FLAG_IGNEMPTY, g.strp);
    if(g.strp == NULL) {
      status = -1;
      log_err(0, "%s", "strsplit_create returned an error.");
      break;
    }

    status = process_stdin_entry();
    if(status != 0)
      break;
  }

  if(g.strp != NULL) {
    strsplit_delete(g.strp);
    g.strp = NULL;
  }

  return(status != 0 ? 1 : 0);
}

static int process_stdin_entry(void) {

  char *optstr = "c:f:gmno:p:s:";
  int status = 0;

  process_args(g.strp->argc, g.strp->argv, optstr, 0);

  if(optind < g.strp->argc - 1)
    log_errx(1, "Too many arguments.");
  else if(optind == g.strp->argc - 1)
    g.input_fname = g.strp->argv[optind++];
  else
      log_errx(1, "Not enough arguments");

  status = process_file();

  return(status != 0 ? 1 : 0);
}

#ifdef TEST
static void test_print(void) {

  fprintf(stdout, "%u : %u\n", g.opt_ccbsize, gdefault.opt_ccbsize);
  fprintf(stdout, "%u : %u\n", g.opt_feedtype, gdefault.opt_feedtype);
  fprintf(stdout, "%d : %d\n", g.opt_gempak, gdefault.opt_gempak);
  fprintf(stdout, "%d : %d\n", g.opt_md5seq, gdefault.opt_md5seq);
  fprintf(stdout, "%d : %d\n", g.opt_noccb, gdefault.opt_noccb);
  fprintf(stdout, "%s : %s\n", g.opt_origin, gdefault.opt_origin);
  fprintf(stdout, "%s :\n", g.opt_prodid);
  fprintf(stdout, "%s : %s\n", g.opt_pqfname, gdefault.opt_pqfname);
  fprintf(stdout, "%s :\n", g.opt_seq_str);
  fprintf(stdout, "%s :\n", g.input_fname);

  fprintf(stdout, "%s\n\n", "=======================");
}
#endif

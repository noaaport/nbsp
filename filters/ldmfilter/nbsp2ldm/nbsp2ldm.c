/*
 * Copyright (c) 2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See the LICENSE file in the nbsp distribution.
 *
 * $Id$
 */
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

#define DEFAULT_PQFNAME		"/var/ldm/ldm.pq"
#define DEFAULT_PRODORIGIN	"nbsp"

#define DEFAULT_SEQNUM          90000
#define DEFAULT_FEEDTYPE	WMO
#define DEFAULT_CCB_SIZE	24

struct {
  unsigned int opt_ccbsize;	/* [-c] */  
  unsigned int opt_feedtype;	/* [-f] */
  int opt_gempak;	/* [-g] add gempak trailer/footer */
  int  opt_md5seq;	/* [-m]  => calculate md5 from [-s] sequence */
  int opt_noccb;	/* [-n] no ccb in file */
  char *opt_origin;	/* [-o] origin */
  char *opt_prodid;	/* [-p] */
  char *opt_pqfname;	/* [-q] */
  char *opt_seq_str;	/* [-s] */
  unsigned int opt_filesize;	/* [-S] => must be given when file is stdin */
  /* variables */
  unsigned int seq;
  char *input_fname; 
  struct pqueue *pq;
  int fd;
} g = {DEFAULT_CCB_SIZE, DEFAULT_FEEDTYPE, 0, 0, 0,
       DEFAULT_PRODORIGIN, NULL,
       DEFAULT_PQFNAME, NULL, 0,
       DEFAULT_SEQNUM, NULL, NULL, -1};

#define GMPK_TRAILER_SIZE	4
#define GMPK_HEADER_SIZE       11
char *gmpk_header_fmt = "\001\r\r\n%03d \r\r\n"; 
char *gmpk_trailer_str = "\r\r\n\003";
char gmpk_header_str[GMPK_HEADER_SIZE + 1];

#define STATIC_POOL_SIZE	1048576
char static_pool[STATIC_POOL_SIZE];

static ssize_t readn(int fd, void *p, size_t n);
static void input_file_errx(int e, char *s);
static void input_file_err(int e, char *s);
static int process_file(void);
static int extract_uint(char *s, unsigned int *val);
static void cleanup(void);
static int calc_md5(unsigned char *buf, off_t bufsize, signaturet signature);

static void input_file_errx(int e, char *s){

  if(g.input_fname == NULL)
    errx(e, "%s", s);
  else 
    errx(e, "%s: %s", s, g.input_fname);
}

static void input_file_err(int e, char *s){

  if(g.input_fname == NULL)
    err(e, "%s", s);
  else 
    err(e, "%s: %s", s, g.input_fname);
}

static ssize_t readn(int fd, void *p, size_t n){
  /*
   * Use when reading from stdin.
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

static void cleanup(void){
  /*
   * If malloc() was used for prod.data, we leave it to the OS to free
   * the memory allocated.
   */
  if(g.pq != NULL) {
    pq_close(g.pq);
    g.pq = NULL;
  }  
  
  if((g.fd != -1) && (g.fd != STDIN_FILENO)){
    close(g.fd);
    g.fd = -1;
  }
}

static int calc_md5(unsigned char *buf, off_t bufsize, signaturet signature){

  MD5_CTX md5;

  MD5_Init(&md5);
  MD5_Update(&md5, buf, bufsize);
  MD5_Final(signature, &md5);

  return(0);
}

int main(int argc, char **argv){
  
  char *optstr = "c:f:gmno:p:q:s:S:";
  char *usage = "nbsp2ldm [-c ccbsize] [-f feedtype] [-g] [-m] [-n]"
    " [-o origin] [-p prodid] [-q pqfname] [-s seq]"
    " <filename> | -S <filesize> < stdin";
  int status = 0;
  int c;

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'c':
      status = extract_uint(optarg, &g.opt_ccbsize);
      if(status != 0)
	errx(1, "Invalid value for [-c].");

      break;
    case 'f':
      status = extract_uint(optarg, &g.opt_feedtype);
      if(status != 0)
	errx(1, "Invalid value for [-f].");

      break;
    case 'g':
      g.opt_gempak = 1;
      break;
    case 'm':
      g.opt_md5seq = 1;
      break;
    case 'n':
      g.opt_noccb = 1;
      break;
    case 'o':
      g.opt_origin = optarg;
      break;
    case 'p':
      g.opt_prodid = optarg;
      break;
    case 'q':
      g.opt_pqfname = optarg;
      break;
    case 's':
      g.opt_seq_str = optarg;
      status = extract_uint(optarg, &g.seq);
      if(status != 0)
	errx(1, "Invalid value for [-s].");

      break;
    case 'S':
      status = extract_uint(optarg, &g.opt_filesize);
      if(status != 0)
	errx(1, "Invalid value for [-S].");

      break;
    case 'h':
    default:
      status = 1;
      errx(1, "%s", usage);
      break;
    }
  }

  if(optind < argc - 1)
    errx(1, "Too many arguments.");
  else if(optind == argc - 1)
    g.input_fname = argv[optind++];
  else if((g.opt_filesize == 0) || (g.opt_prodid == NULL))
    errx(1, "Options [-S] and [-p] are mandatory when reading from stdin.");

  if(atexit(cleanup) != 0)
    err(1, "atexit");

  status = process_file();

  return(status != 0 ? 1 : 0);
}

static int process_file(void){

  struct stat statb;
  int ccb_offset = g.opt_ccbsize;
  int status = 0;
  product prod;
  ssize_t nread;
  ssize_t filedata_size;
  unsigned char *p;
  int n;
  
  status = pq_open(g.opt_pqfname, PQ_DEFAULT, &g.pq);
  if(status != 0){
    if(status == PQ_CORRUPT)
      errx(1, "The product queue %s is inconsistent.", g.opt_pqfname);
    else 
      err(1, "pq_open");
  }

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
    errx(1, "No prodid specified.");

  if(g.input_fname == NULL){
    g.fd = STDIN_FILENO;
    filedata_size = g.opt_filesize;
  }else {
    g.fd = open(g.input_fname, O_RDONLY, 0);
    if(g.fd == -1)
      err(1, "open");
	
    if(g.opt_filesize == 0){
      if(fstat(g.fd, &statb) == -1)
	err(1, "fstat");
      
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
  else
    prod.data = malloc(prod.info.sz);

  if(prod.data == NULL)
    err(1, "malloc");

  if(ccb_offset != 0){
    if(lseek(g.fd, ccb_offset, SEEK_SET) == -1)
      input_file_err(1, "lseek error in inputfile");
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
    input_file_err(1, "read() error");

  if((g.opt_seq_str != NULL) && (g.opt_md5seq == 1))
    status = calc_md5((unsigned char*)g.opt_seq_str, strlen(g.opt_seq_str),
		      prod.info.signature);
  else
    status = calc_md5(p, filedata_size, prod.info.signature);

  if (status != 0)
    errx(1, "calc_md5");

  if(g.opt_gempak == 1){
    p += filedata_size;
    memcpy(p, gmpk_trailer_str, GMPK_TRAILER_SIZE);
  }

  status = pq_insert(g.pq, &prod);    
  if(status == PQUEUE_DUP){
    status = 0;
    warnx("Product already in queue: %s.", prod.info.ident);
  }else if(status != 0)
    errx(1, "pq_insert: %d", status);

  return(status);
}

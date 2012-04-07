/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

/*
 * Same as wr except using a cache-only db.
 *
 * Make a directory "in" and a copy a bunch of files from the spool
 * directory (and "mkdir db" as well). Then execute
 *
 * ./wr in/*
 *
 * The files will be stored in the spoolbdb, then read and written in the
 * current directory.
 */

#include <sys/stat.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "spoolbdbv.h"

static struct spoolbdbv_st *gdbv = NULL;
static struct spoolbuf_st *gspoolb = NULL;
  
void log_info(char *str){

  fprintf(stderr, "%s\n", str);
}

int spool_open(char *dbhome, int dbenv_mode, uint32_t dbcache_mb,
		      unsigned int Ndb, uint32_t nmax,
		      char *dbfname, int dbfile_mode, int f_mpool_nofile){

  int status;

  /* The db must be created, f_rdonly = 0 as last argument */
  status = spoolbdbv_open(&gdbv, dbhome, dbenv_mode, dbcache_mb,
			 Ndb, nmax, dbfname, dbfile_mode, f_mpool_nofile, 0);

  if(status != 0)
    errx(1, "Cannot open data base. %s", db_strerror(status));

  return(status);
}

void spool_close(void){

  int status;

  if(gdbv == NULL)
    return;

  status = spoolbdbv_close(gdbv);
  gdbv = NULL;

  if(status != 0)
    errx(1, "Error closing db. %s", db_strerror(status));
  else
    log_info("Closed db env.");
}

int get_file_size(char *fname, off_t *fsize){

  struct stat sb;
  int status = 0;
  
  status = stat(fname, &sb);
  if(status == 0)
    *fsize = sb.st_size;

  return(status);
}

int write_file(char *fname){

  int status = 0;
  char *p;
  int fd;
  off_t fsize;
  char *fkey;

  status = get_file_size(fname, &fsize);
  if(status != 0)
    err(1, "get_file_size");

  if(fsize > gspoolb->max_buffer_size){
    p = malloc(fsize);
    if(p == NULL)
      err(1, "malloc");
    else{
      free(gspoolb->buffer);
      gspoolb->buffer = p;
      gspoolb->max_buffer_size = fsize;
    }
  }

  gspoolb->buffer_size = fsize;
  fd = open(fname, O_RDONLY);
  if(fd == -1)
    err(1, "open");

  if(read(fd, gspoolb->buffer, gspoolb->buffer_size) == -1)
    err(1, "read");

  close(fd);

  fkey = strchr(fname, '.');
  ++fkey;
  status = spoolbdbv_write(gdbv, fkey, gspoolb);
  if(status != 0)
    warnx(db_strerror(status));

  return(status);
}

int write_files(char **files){

  int status = 0;
  char **f;

  f = files;
  while(*f != NULL){
    fprintf(stdout, "writing: %s\n", *f);
    status = write_file(*f);
    ++f;
  }

  return(status);
}

int read_file(char *fname){

  int status = 0;
  int fd;
  char *fkey;

  fkey = strchr(fname, '.');
  ++fkey;
  status = spoolbdbv_read(gdbv, fkey, gspoolb);
  if(status == DB_NOTFOUND){
    warnx("not found: %s", &fname[3]);
    return(0);
  }

  if(status != 0)
    errx(1, db_strerror(status));

  fd = open(&fname[3], O_WRONLY | O_CREAT, 0644);
  if(fd == -1)
    err(1, "open");

  if(write(fd, gspoolb->buffer, gspoolb->buffer_size) == -1)
    err(1, "read");

  close(fd);


  return(status);
}

int read_files(char **files){

  int status = 0;
  char **f;

  f = files;
  while(*f != NULL){
    fprintf(stdout, "reading: %s\n", *f);
    status = read_file(*f);
    ++f;
  }

  return(status);
}

static void cleanup(void){

  if(gspoolb != NULL){
    spoolbuf_destroy(gspoolb);
    gspoolb = NULL;
  }

  spool_close();
}

int main(int argc, char **argv){

  int status = 0;
  int Ndb = 4;
  int nmax = 7;
  uint32_t dbcache_mb = 64;
  int dbfile_mode = 0644;
  int dbenv_mode = 0644;
  char *dbhome = "db";	
  char *dbfname = "spooldb";
  int f_mpool_nofile = 1;
  char **files;

  atexit(cleanup);

  gspoolb = spoolbuf_create();
  if(gspoolb == NULL)
    err(1, "spoolbuf_create");

  status = spool_open(dbhome, dbenv_mode, dbcache_mb,
		      Ndb, nmax, dbfname, dbfile_mode, f_mpool_nofile);

  if(status != 0)
    errx(1, "spoolbdbv_open: %s", db_strerror(status));

  files = argv;
  ++files;
  status = write_files(files);
  if(status == 0)
    status = read_files(files);

  return(0);
}

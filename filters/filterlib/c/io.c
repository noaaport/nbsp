/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include "err.h"
#include "io.h"

static char *s_input_filename = NULL;
static char *s_output_filename = NULL;

FILE *fopen_input(char *path){

  FILE *f;

  if(path == NULL)
    return(stdin);

  f = fopen(path, "r");
  if(f == NULL){
    log_err_open(path);
    return(NULL);
  }

  s_input_filename = path;

  return(f);
}

FILE *fopen_output(char *path, char *mode){

  FILE *f;
  int fd;

  if(path == NULL)
    return(stdout);

  /*
   * In FreeBSD we could use O_EXLOCK and not use flock below,
   * but not in general (e.g., linux).
   */
  if(mode[0] == 'a')
    fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0666);
  else if(mode[0] == 'w')
    fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, 0666);
  else{
    log_errx(1, "Invalid mode %s.", mode);
    return(NULL);
  }

  if(fd != -1){
    if(flock(fd, LOCK_EX) == -1){
      (void)close(fd);
      fd = -1;
    }
  }

  if(fd == -1){
    log_err_open(path);
    return(NULL);
  }

  f = fdopen(fd, mode);
  if(f == NULL){
    (void)close(fd);
    log_err(1, "Error from fdopen");
  }

  s_output_filename = path;

  return(f);
}

int read_page(FILE *fp, void *page, int page_size){

  int n;

  n = fread(page, 1, page_size, fp);
  if(n < page_size){
    if(ferror(fp) != 0){
      n = -1;
      log_err_read(s_input_filename);
    }
  }

  return(n);
}

int write_page(FILE *fp, void *page, int page_size){

  int n;

  n = fwrite(page, 1, page_size, fp);
  if(n < page_size){
    n = -1;
    log_err_write(s_output_filename);
  }
  
  return(n);
}

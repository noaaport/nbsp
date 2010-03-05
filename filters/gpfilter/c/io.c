/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include "err.h"
#include "io.h"

int read_page(FILE *fp, void *page, int page_size){

  int n;

  n = fread(page, 1, page_size, fp);
  if(n < page_size){
    if(ferror(fp) != 0){
      n = -1;
      log_err(1, "Cannot read input.");
    }
  }

  return(n);
}

int write_page(FILE *fp, void *page, int page_size){

  int n;

  n = fwrite(page, 1, page_size, fp);
  if(n < page_size){
    n = -1;
    log_err(1, "Cannot write output.");
  }
  
  return(n);
}

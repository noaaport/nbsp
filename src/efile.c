/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include "err.h"
#include <limits.h>
#include <stdio.h>
#include "file.h"
#include "efile.h"
#include "globals.h"

static void log_open_file_error(char *fname, int status);

int e_open_product_file(struct pctl_element_st *pce){
  /*
   * Returns the fd or -1 on error.
   */
  int flags = O_CREAT | O_WRONLY | O_TRUNC;
  int fd = -1;
  int status = 0;

  status = create_path_dirs(pce->fpath, g.subdir_product_mode);

  if(status == 0){
    fd = open(pce->fpath, flags, g.product_mode);
    if(fd == -1)
      status = -1;
  }

  if(status != 0)
    log_open_file_error(pce->fpath, status);

  return(fd);
}

static void log_open_file_error(char *fname, int status){
  /*
   * If status == -1 we assume it was a OS error; if it is 1,
   * we assume it comes from one of the make_X_name() functions.
   */
  if(status == 1)
    log_errx("Could not open %s. Name too long.", fname);
  else if(status == -1)
    log_err2("Could not open", fname);
}

int e_dir_exists(char *dirname){

  int status;

  status = dir_exists(dirname);

  if(status == -1)
    log_err2("Error checking %s.", dirname);
  else if(status == 1)
    log_errx("%s is not a directory.", dirname);
  else if(status == 2)
    log_errx("%s does not exist.", dirname);

  return(status);
}

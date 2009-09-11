/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/stat.h>
#include <errno.h>
#include "file.h"

static int make_dir_tree2(char *path, mode_t mode);

int file_delete(char *fname){
  /*
   * Returns:
   *
   *	 0 => file deleted or file did not exist
   *	-1 => files could not be deleted
   */
  int status = 0;

  status = file_exists(fname);

  if(status == 0)
    status = unlink(fname);
  else if(status == 1)
    status = 0;

  return(status);
}

int file_exists(char *fname){
  /* 
   * Returns:
   * 0 == > ok. file exists
   * 1 ==> file does not exist
   * -1 ==> error from systat different from ENOENT
   */
  struct stat stbuf;
  int status = 0;

  if(stat(fname, &stbuf) == -1){
    if(errno == ENOENT)
      status = 1;
    else
      status = -1;
  }

  return(status);
}

int dir_exists(char *dirname){
  /* 
   * Returns:
   * 0 == > ok. dir exists
   * 1 ==> dir does not exist
   * 2 ==> name exists but is not a directory
   * -1 ==> error from systat different from ENOENT
   */
  struct stat stb;
  int status = 0;

  if(stat(dirname, &stb) == -1){
    if(errno == ENOENT)
      status = 1;
    else
      status = -1;
  }else if(S_ISDIR(stb.st_mode) == 0)
      status = 2;

  return(status);
}

int get_file_size(char *fname, off_t *fsize){

  struct stat sb;
  int status = 0;
  
  status = stat(fname, &sb);
  if(status == 0)
    *fsize = sb.st_size;

  return(status);
}

int make_dir_tree(char *path, mode_t mode){

  int status = 0;
  char *p;
  char *s;
  char *path_copy;

  /*
   * The idea is to walk through the path, changing temporarily
   * each '/' (except the first, if any) by a '\0', and call mkdir() 
   * with the resulting path. We make a copy of path in case the
   * path is a constant string.
   */

  if((path == NULL) || (strlen(path) == 0))
    return(1);

  path_copy = malloc(strlen(path) + 1);
  if(path_copy == NULL)
    return(-1);

  strncpy(path_copy, path, strlen(path) + 1);
  p = path_copy;

  while((status == 0) && (p != NULL)){
    if(p[0] == '/')
      ++p;

    s = strchr(p, '/');
    if(s != NULL)
      *s = '\0';

    status = mkdir(path_copy, mode);
    /*
     * If the directory exists, we don't treat it as an error.
     */
    if((status != 0) && (dir_exists(path_copy) == 0))
	status = 0;

    if(s != NULL)
      *s = '/';
    
    p = s;
  }

  free(path_copy);

  return (status);
}

static int make_dir_tree2(char *path, mode_t mode){
  /*
   * Same as above but assume that path can be modified. This
   * is meant to be called by create_path_dirs() below.
   */
  int status = 0;
  char *p;
  char *s;

  /*
   * The idea is to walk through the path, changing temporarily
   * each '/' (except the first, if any) by a '\0', and call mkdir() 
   * with the resulting path. 
   */

  if((path == NULL) || (strlen(path) == 0))
    return(1);

  p = path;

  while((status == 0) && (p != NULL)){
    if(p[0] == '/')
      ++p;

    s = strchr(p, '/');
    if(s != NULL)
      *s = '\0';

    status = mkdir(path, mode);
    /*
     * If the directory exists, we don't treat it as an error.
     */
    if((status != 0) && (dir_exists(path) == 0))
	status = 0;

    if(s != NULL)
      *s = '/';
    
    p = s;
  }

  return (status);
}

int create_path_dirs(char *fname, mode_t mode){

  int status;
  char *p;
  char *fname_copy;

  /* 
   * Check, with the the last ocurrence of '/', if there is directory part
   */
  p = strrchr(fname, '/');
  if(p == NULL)
    return(0);

  /*
   * Work on a copy since some implementations of dirname() modify
   * the argument (e.g., linux).
   */
  fname_copy = malloc(strlen(fname) + 1);
  if(fname_copy == NULL)
    return(-1);
  
  memcpy(fname_copy, fname, strlen(fname) + 1);
  p = dirname(fname_copy);
  status = make_dir_tree2(p, mode);
  free(fname_copy);

  return(status);
}

int append_file(int fd_to, int fd_from){
  /*
   * Returns:
   *
   *	-1 => read error
   *	-2 => write error
   *	 0 => no error
   */
  int status = 0;
  char buf[1024];
  int n;

  do{
    n = read(fd_from, buf, 1024);
    if(n == -1){
      status = -1;
    }else if(n > 0){
      if(write(fd_to, buf, n) == -1)
	status = -2;
    }
  }while((n > 0) && (status == 0));

  return(status);
}

char *findbasename(char *path){
  /*
   * Find the last "/" and return a pointer to the character after it.
   * Returns NULL if path is NULL, or empty, or ends with a "/".
   */
  char *p;

  if(path == NULL || *path == '\0')
    return(NULL);
    
  p = strrchr(path, '/');
  if(p == NULL)
    return(path);

  ++p;
  if(*p == '\0')
    return(NULL);
      
  return(p);
}

char *make_temp_logfile(char *logfile, char *ext){
/*
 * Logfiles that are meant to be read by other programs will be written
 * to a temporary file with the additional extension, and then copied
 * to the "real" file using rename(). The caller must call free() on the
 * result returned by this function.
 */
  size_t size;
  char *p;
  
  size = strlen(logfile) + strlen(ext) + 1;
  p = malloc(size);
  if(p == NULL)
    return(NULL);
  
  strncpy(p, logfile, size);
  strncat(p, ext, strlen(ext));

  return(p);
}

#if 0
/*
 * Test of make_dir_tree()
 */
#include <err.h>
int main(int argc, char **argv){

  int status = 0;

  if(argc != 2)
    errx(1, "argc");

  status = make_dir_tree(argv[1], 0755);

  if(status != 0)
    warn("make_dir_tree()");

  return(0);
}
#endif

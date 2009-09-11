/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef FILE_H
#define FILE_H

#include <fcntl.h>

int file_delete(char *fname);
int file_exists(char *fname);
int get_file_size(char *fname, off_t *fsize);
int dir_exists(char *dirname);
int make_dir_tree(char *path, mode_t mode);
int create_path_dirs(char *fname, mode_t mode);
int append_file(int fd_to, int fd_from);
char *findbasename(char *path);
char *make_temp_logfile(char *logfile, char *ext);

#endif

/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * $Id$
 */
#ifndef SBNPACK_PRIVATE_H
#define SBNPACK_PRIVATE_H

#include "sbnpack.h"

int init_sbnpack_file(char *fname, struct sbnpack_file_st *sbnpack_file);
int reinit_sbnpack_file(char *fname, struct sbnpack_file_st *sbnpack_file);
void free_sbnpack_file(struct sbnpack_file_st *sbnpack_file);

struct sbnpack_frame_st *create_sbnpack_frame_array(int nframes,
						    int last_frame_size);
void destroy_sbnpack_frame_array(struct sbnpack_frame_st *sbnpack_frame);

/* in fill.c */
void fill_blockdata(struct sbnpack_st *sbnpack);
void fill_headers(struct sbnpack_st *sbnpack);
void fill_headers_test(struct sbnpack_st *sbnpack);

#endif

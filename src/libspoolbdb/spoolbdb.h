/*
 * Copyright (c) 2006-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SPOOLBDB_H
#define SPOOLBDB_H

#include <inttypes.h>

struct spoolbuf_st {
  void *buffer;
  uint32_t buffer_size;
  uint32_t max_buffer_size;
  char *readp;		/* reading pointer */
  uint32_t nread;	/* amount already read */
};

struct spoolbuf_st *spoolbuf_create(void);
void spoolbuf_destroy(struct spoolbuf_st *spoolbuf);

void *spoolbuf_data(struct spoolbuf_st *);
size_t spoolbuf_datasize(struct spoolbuf_st *);
size_t spoolbuf_maxsize(struct spoolbuf_st *);

size_t spoolbuf_read(struct spoolbuf_st *spoolb,
		      void *data, size_t size);
void spoolbuf_read_init(struct spoolbuf_st *spoolb);

#endif

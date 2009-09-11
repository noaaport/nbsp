/*
 * Copyright (c) 2005-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <sys/types.h>

#ifndef PFILTER
#define PFILTER

struct pfilter_st {
  FILE *fp;
  int fifofd;
  char *fifofpath;
  unsigned int secs;
};

#define pfilter_vprintf fprintf

struct pfilter_st *pfilter_open(char *script);
struct pfilter_st *pfilter_open_wr(char *script, char *fifo,
				   unsigned int secs);

void pfilter_close(struct pfilter_st *filterp);
int pfilter_read(struct pfilter_st *filterp, void *buffer, size_t size);
int pfilter_flush(struct pfilter_st *filterp);

#endif

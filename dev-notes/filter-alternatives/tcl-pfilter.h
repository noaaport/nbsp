/*
 * Copyright (c) 2005-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <sys/types.h>
#include <tcl.h>

#ifndef PFILTER
#define PFILTER

#define PFILTER_FLAGS (TCL_STDIN | TCL_STDOUT | TCL_STDERR)
#define PRINTF_BUFFER_SIZE_GROW_FACTOR	2
#define PRINTF_BUFFER_SIZE_INIT		2

struct pfilter_st {
  Tcl_Interp *interp;
  Tcl_Channel channel;
  char *printf_buffer;
  int printf_buffer_size;
  int fifofd;
  char *fifofpath;
  unsigned int secs;
};

struct pfilter_st *pfilter_open(char *script);
struct pfilter_st *pfilter_open_wr(char *script, char *fifo,
				   unsigned int secs);

void pfilter_close(struct pfilter_st *filterp);
int pfilter_write(struct pfilter_st *filterp, void *buffer, size_t size);
int pfilter_vprintf(struct pfilter_st *filterp, char *format, ...);
int pfilter_read(struct pfilter_st *filterp, void *buffer, size_t size);
int pfilter_flush(struct pfilter_st *filterp);

#endif

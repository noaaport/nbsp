/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef TCLHTTPD_H
#define TCLHTTPD_H

#include <unistd.h>

struct tclhttpd_st {
  char *script;
  char *fifo;
  int child_status;	/* 0 => created */
  pid_t child_pid;
};

struct tclhttpd_st *tclhttpd_open(char *script, char *fifo);
void tclhttpd_close(struct tclhttpd_st *hp);

#endif

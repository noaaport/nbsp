/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef FILTERS_H
#define FILTERS_H

#include <time.h>
#include <limits.h>
#include <dirent.h>
#include "pfilter.h"

/* The types of filters */
#define FILTER_TYPE_NONE	0
#define FILTER_TYPE_FIFO	1
#define FILTER_TYPE_PIPE	2
#define FILTER_TYPE_EVAL	3	/* embedded tcl */

struct filter_stats {
  unsigned int files_sent;	/* files sent to the filter */
  unsigned int errors;		/* errors from sendto_one_filter */
};

struct filter_entry_st {
  int fd;
  struct pfilter_st *pfp;
  int type;			/* fifo, pipe, ... */
  char *fname;
  struct filter_stats stats;
};

struct filter_list_st {
  int n;			/* number of occupied slots */
  int nmax;			/* available slots */
  struct filter_entry_st *filters;
};

int spawn_filter_server(void);
void kill_filter_thread(void);
void set_reload_filters_flag(void);

void set_filterserver_state_flag(void);

#endif

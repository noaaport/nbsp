/*
 * Copyright (c) 2006-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SPOOLSLOTS_H
#define SPOOLSLOTS_H

#include <pthread.h>
#include "spoolbdbv.h"
#include "spoolbdb.h"

struct spoolbuf_slots_st {
  int numslots;
  int maxslots;
  struct spoolbuf_st **spoolb;  /* array of pointers */
  int *status;                  /* 0,1 -> closed,open */
  pthread_cond_t  cond;		/* wait for empty slot */
  pthread_mutex_t mutex;	/* control open/close access (not read) */
};

struct spoolbuf_slots_st *spoolbuf_slots_create(int N);
void spoolbuf_slots_destroy(struct spoolbuf_slots_st *spoolslots);
int spoolbuf_slots_open(struct spoolbuf_slots_st *spoolslots,
			struct spoolbdbv_st *spooldb, char *fkey, int *sd);
int spoolbuf_slots_close(struct spoolbuf_slots_st *, int sd);
int spoolbuf_slots_open_unlocked(struct spoolbuf_slots_st *spoolslots,
				 struct spoolbdbv_st *spooldb, char *fkey,
				 int *sd);
int spoolbuf_slots_close_unlocked(struct spoolbuf_slots_st *spoolslots,
				  int sd);
size_t spoolbuf_slots_read(struct spoolbuf_slots_st *, int sd,
			    void *data, size_t size);
size_t spoolbuf_slots_datasize(struct spoolbuf_slots_st *, int sd);

#endif

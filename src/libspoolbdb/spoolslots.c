/*
 * Copyright (c) 2006-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include "spoolslots.h"

static int _spoolbuf_slots_open_unlocked(struct spoolbuf_slots_st *spoolslots,
					 struct spoolbdbv_st *spooldb,
					 char *fkey, int *sd,
					 int f_read_unlocked);

struct spoolbuf_slots_st *spoolbuf_slots_create(int N){

  int status = 0;
  int i;

  assert(N > 0);

  if(N <= 0){
    errno = EINVAL;
    return(NULL);
  }

  struct spoolbuf_slots_st *spoolslots = NULL;

  spoolslots = malloc(sizeof(struct spoolbuf_slots_st));
  if(spoolslots == NULL){
    return(NULL);
  }

  spoolslots->numslots = 0;
  spoolslots->maxslots = 0;
  spoolslots->spoolb = NULL;
  spoolslots->status = NULL;

  status = pthread_mutex_init(&spoolslots->mutex, NULL);
  if(status == 0){
    status = pthread_cond_init(&spoolslots->cond, NULL);
    if(status != 0)
      pthread_mutex_destroy(&spoolslots->mutex);
  }

  if(status != 0){
    free(spoolslots);
    return(NULL);
  }

  spoolslots->status = calloc(N, sizeof(int));
  if(spoolslots->status == NULL){
    free(spoolslots);
    return(NULL);
  }
  for(i = 0; i < N; ++i)
    spoolslots->status[i] = 0;

  spoolslots->spoolb = calloc(N, sizeof(struct spoolbuf_st*));
  if(spoolslots->spoolb == NULL){
    spoolbuf_slots_destroy(spoolslots);
    return(NULL);
  }
  for(i = 0; i < N; ++i)
    spoolslots->spoolb[i] = NULL;

  spoolslots->maxslots = N;

  for(i = 0; i < N; ++i){
    spoolslots->spoolb[i] = spoolbuf_create();
    if(spoolslots->spoolb[i] == NULL){
      status = -1;
      break;
    }
  }

  if(status != 0){
    spoolbuf_slots_destroy(spoolslots);
    return(NULL);
  }

  return(spoolslots);
}

void spoolbuf_slots_destroy(struct spoolbuf_slots_st *spoolslots){

  int i;

  assert(spoolslots != NULL);

  pthread_mutex_destroy(&spoolslots->mutex);
  pthread_cond_destroy(&spoolslots->cond);

  if(spoolslots->status != NULL)
    free(spoolslots->status);

  if(spoolslots->spoolb != NULL){
    for(i = 0; i < spoolslots->maxslots; ++i){
      if(spoolslots->spoolb[i] != NULL){
	spoolbuf_destroy(spoolslots->spoolb[i]);
      }
    }
    free(spoolslots->spoolb);
  }

  free(spoolslots);
}

int spoolbuf_slots_open(struct spoolbuf_slots_st *spoolslots,
			struct spoolbdbv_st *spooldb, char *fkey, int *sd){
  int status = 0;
  int status1 = 0;

  status = pthread_mutex_lock(&spoolslots->mutex);
  if(status != 0)
    return(status);

  while((spoolslots->numslots == spoolslots->maxslots) && (status == 0)){
    status = pthread_cond_wait(&spoolslots->cond, &spoolslots->mutex);
  }
  if(status != 0){
    pthread_mutex_unlock(&spoolslots->mutex);
    return(status);
  }

  /*
   * There is a slot available. Passing 0 in the last argument makes
   * the function use the locking version of spoolbdbv_read(); for example
   * as required by a program using the mspoolbdb.
   */
  status = _spoolbuf_slots_open_unlocked(spoolslots, spooldb, fkey, sd, 0);

  status1 = pthread_mutex_unlock(&spoolslots->mutex);
  if(status == 0)
    status = status1;

  return(status);  
}

int spoolbuf_slots_close(struct spoolbuf_slots_st *spoolslots, int sd){
  
  int status = 0;
  int status1 = 0;

  status = pthread_mutex_lock(&spoolslots->mutex);
  if(status != 0)
    return(status);

  (void)spoolbuf_slots_close_unlocked(spoolslots, sd);

  status = pthread_cond_signal(&spoolslots->cond);
  status1 = pthread_mutex_unlock(&spoolslots->mutex);
  
  if(status == 0)
     status= status1;

  return(status);
}

size_t spoolbuf_slots_read(struct spoolbuf_slots_st *spoolslots, int sd,
			    void *data, size_t size){
  /*
   * This function is meant to be called by only one thread, the one
   * to which the "sd" belongs, and therefore it is not mutex protected.
   *
   * Note that the return value from this function is different from
   * the others.
   */
  return(spoolbuf_read(spoolslots->spoolb[sd], data, size));
}

size_t spoolbuf_slots_datasize(struct spoolbuf_slots_st *spoolslots, int sd){
  /*
   * Similar to the read function above, this function is meant to be used
   * by only on thread, the one that opened the slot.
   */

  return(spoolbuf_datasize(spoolslots->spoolb[sd]));
}

/*
 * Unlocked versions of open and close. These versions can be used by
 * programs in which only one thread read from the bdb, and open only
 * one slot at a any given time. Otherwise the locked versions must
 * be used, which call these ones but properly protected.
 */
static int _spoolbuf_slots_open_unlocked(struct spoolbuf_slots_st *spoolslots,
					 struct spoolbdbv_st *spooldb,
					 char *fkey, int *sd,
					 int f_read_unlocked){
  int status = 0;
  int i;

  /*
   * - Find the first available slot.
   * - Fill the spoolbuf from the database data.
   * - Update the spoolslots parameters
   */
  for(i = 0; i < spoolslots->maxslots; ++i){
    if(spoolslots->status[i] == 0)
      break;
  }
  assert(i < spoolslots->maxslots);

  if(f_read_unlocked)
    status = spoolbdbv_read_unlocked(spooldb, fkey, spoolslots->spoolb[i]);
  else
    status = spoolbdbv_read(spooldb, fkey, spoolslots->spoolb[i]);

  if(status == 0){
    *sd = i;
    ++spoolslots->numslots;
    spoolslots->status[i] = 1;
  }

  return(status);  
}

int spoolbuf_slots_open_unlocked(struct spoolbuf_slots_st *spoolslots,
				 struct spoolbdbv_st *spooldb, char *fkey,
				 int *sd){

  return(_spoolbuf_slots_open_unlocked(spoolslots, spooldb, fkey, sd, 1));
}

int spoolbuf_slots_close_unlocked(struct spoolbuf_slots_st *spoolslots,
				  int sd){
  
  int status = 0;

  --spoolslots->numslots;
  spoolslots->status[sd] = 0;

  return(status);
}

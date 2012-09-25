/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include "globals.h"
#include "err.h"
#include "signal.h"
#include "npcast.h"
#include "framep.h"
#include "reader.h"

struct reader_info_st {
  int channel_index;
  struct sbn_frame sbnframe;
};
static struct reader_info_st greader_info[NPCAST_NUM_CHANNELS];

static int create_reader_thread(int channel_index);
static void *reader_thread_main(void *arg);
static int loop_channel(int channel_index);
static int reader_thread_loop(int channel_index);

static void load_ave_cond_sleep(int channel_index);

int spawn_readers(void){
  /*
   * Spawn one thread for each noaaport channel enabled.
   */
  int i;
  int status = 0;
  int num_readers = NPCAST_NUM_CHANNELS;

  for(i = 0; i < num_readers; ++i)
    greader_info[i].channel_index = i;

  for(i = 0; i < num_readers; ++i){
    if(get_npcast_channel_enable(i))
      status = create_reader_thread(i);
      
    if(status != 0)
      break;
  }

  return(status);
}

void kill_reader_threads(void){
  /*
   * This "joins" the reader threads. Normally they quit
   * in their loop when they check the quit flag. But if a reader
   * is waiting on a channel without data, it can wait for the
   * g.brodcast_read_timeout_secs timeout and that may be too long.
   * Therefore we "cancel" them. It is possible that some of them
   * have already quit by the time we send them the cancelation request,
   * and in that case pthread_cancel returns ESRCH. To avoid that they
   * are canceled while they hold a mutex locked, the cancellation state
   * is set to DISABLED and renabled in the main loop_channel() around the
   * appropriate portion (the sbnframeproc function).
   */
  int i;
  int num_readers = NPCAST_NUM_CHANNELS;
  int status;
  void *pthread_status;

  for(i = 0; i < num_readers; ++i){
    if(g.f_reader_thread_created[i] == 1){

      /* log_info("Canceling reader %d.", i); */

      status = pthread_cancel(g.reader_thread_id[i]);
      if((status != 0) && (status != ESRCH))
	log_errx("Error %d canceling reader %d.", status, i);
    }
  }

  for(i = 0; i < num_readers; ++i){
    if(g.f_reader_thread_created[i] == 1){
      status = pthread_join(g.reader_thread_id[i], &pthread_status);
      if(status != 0)
	log_errx("Error %d joining reader %d.", status, i);
      else if(pthread_status == PTHREAD_CANCELED)
	log_info("Canceled reader %d.", i);
      else if(pthread_status == NULL)
	log_info("Finished reader %d.", i);
    }
  }
}

static int create_reader_thread(int i){

  int status = 0;
  pthread_t t_id;
  pthread_attr_t attr;
  void *arg;

  arg = (void*)&greader_info[i];

  status = pthread_attr_init(&attr);

  if(status == 0)
    status = pthread_create(&t_id, &attr, reader_thread_main, arg);

  if(status != 0){
    log_err("Cannot create reader thread.");
  }else{
    g.reader_thread_id[i] = t_id;
    g.f_reader_thread_created[i] = 1;
    log_info("Spawned reader thread %d.", i);
  }

  return(status);
}

static void *reader_thread_main(void *arg){

  int status;
  struct reader_info_st *rinfo = (struct reader_info_st*)arg;

  while(get_quit_flag() == 0){
    status = reader_thread_loop(rinfo->channel_index);
    if(status != 0){
      /*
       * If there was an error, this (main thread loop) continues
       */
      ;
    } 
  }

  return(NULL);
}

static int reader_thread_loop(int i){

  int status = 0;

  while((status == 0) && (get_quit_flag() == 0)){
    status = loop_channel(i);
  }

  return(status);
}

static int loop_channel(int channel_index){

  ssize_t n;
  int status = 0;
  int cancel_state;
  struct sbn_frame *sbnf = &greader_info[channel_index].sbnframe;

  pthread_testcancel();
  load_ave_cond_sleep(channel_index);

  /*
  n = recvfrom_channel_nowait(channel_index, sbnf->rawdata, SBN_FRAME_SIZE);
  if(n <= 0)
    n = recvfrom_channel_timed(channel_index, sbnf->rawdata, SBN_FRAME_SIZE,
			       g.broadcast_read_timeout_secs);
  */

  /*
   * Calling it directly seems to work better with the dvb-s2
   * Sat May 14 19:33:28 AST 2011
   */
  n = recvfrom_channel_timed(channel_index, sbnf->rawdata, SBN_FRAME_SIZE,
			     g.broadcast_read_timeout_secs);

  if(n == -1){
    log_err2u("Error reading from noaaport channel", 
	      (unsigned int)channel_index);
    return(-1);
  }else if(n == 0){
    log_info("Timed out reading from channel %d", channel_index);
    return(0);
  }

  sbnf->rawdata_size = n;
  sbnf->np_channel_index = channel_index;

  (void)pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cancel_state);
  status = sbnframeproc(sbnf);
  (void)pthread_setcancelstate(cancel_state, &cancel_state);

  return(status);
}

static void load_ave_cond_sleep(int channel_index){

  if(g.f_loadave_max == 2){
    log_warnx("High load condition. Sleeping reader %d.", channel_index);
    sleep(g.loadave_max_sleep_secs);
    log_warnx("Waking up reader %d.", channel_index);
  }
}

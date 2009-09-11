/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

/*
 * This file substitutes nbspq.c if the libqdb library is used instead
 * of libq. The only differences are in:
 * 
 * q_log_err() [strerror vs db_strerror]
 * init_nbspq() [a different calling sequence to nbspqt_open()]
 */
#include <assert.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include "err.h"
#include "util.h"
#include "packfp.h"
#include "globals.h"
#include "const.h"
#include "libqdb/qdb.h"
#include "dbstats.h"
#include "nbspq.h"

/* 
 * For keeping track of soft and hard limit breaking.
 * There is one such struc for each queue type (channel),
 * stored in the "appdata" field of the qtable.
 */
/*
struct queue_state_st {
  int status;
  time_t last_rept;
};
*/
struct queue_state_st {
  int current;
  int last;
};

static void q_log_err(char *s, int qerrno);
static void nbspq_set_quota_status1(int type, int status);
static void e_nbspq_quota1(int type);

static void q_log_err(char *s, int qerrno){

  log_err_db(s, qerrno);
}

int init_nbspq(void){

  struct queue_state_st *qstate = NULL;
  nbspqtable_t  *qt = NULL;
  int status = 0;
  int qerrno;
  uint32_t data_size;
  struct qdb_param_st qdbparam;
  int i;

  qstate = calloc(g.queue_numchannels, sizeof(struct queue_state_st));
  if(qstate == NULL){
    status = -1;
    qerrno = errno;
  }
  for(i = 0; i < g.queue_numchannels; ++i){
    qstate[i].current = 0;
    qstate[i].last = 0;
  }

  /*
   * This function computes the maximum value that a queued packet will
   * have, based on the convention used for determining the fpath of the
   * file. This will be the record length of the queue db.
   */
  data_size = const_fpath_packet_maxsize();

  qdbparam.dbenv = g.dbenv;
  qdbparam.dbname = NULL;
  if(valid_str(g.qdb_dbfname))
    qdbparam.dbname = g.qdb_dbfname;

  qdbparam.mode = g.dbfile_mode;
  qdbparam.extent_size = g.dbextent_size;
  qdbparam.reclen = data_size;

  if(status == 0){
    qt = nbspqt_open(g.queue_numchannels,
		     g.queue_maxsize_soft, g.queue_maxsize_hard,
		     &qdbparam, &qerrno);
    if(qt == NULL){
      status = -1;
      free(qstate);
    }
  }

  if(status != 0)
    q_log_err("Could not create queue table.", qerrno);
  else{
    qt->appdata = (void*)qstate;
    g.qtable = qt;
  }

  return(status);
}

void kill_nbspq(void){

  int status;
  int dberror;

  void *appdata = NULL;

  if(g.qtable != NULL)
    appdata = g.qtable->appdata;

  if(appdata != NULL)
    free(appdata);

  status = nbspqt_close(g.qtable, &dberror);
  g.qtable = NULL;

  if(status != 0)
    q_log_err("Error closing qtable.", dberror);
  else
    log_info("Closed qdb.");
}

int e_nbspq_snd_filter(void *packet, size_t packet_size){
  /*
   * If the filter server is disabled, no packets should
   * be sent to its queue.
   */
  int status = 0;

  if(g.f_filter_thread_created)
      status = e_nbspq_snd1(FILTER_Q_INDEX, packet, packet_size);

  return(status);
}

int e_nbspq_snd_server(void *packet, size_t packet_size){
  /*
   * If the network server is disabled, no packets should
   * be sent to its queue.
   */
  int status = 0;

  if(g.f_server_thread_created)
    status = e_nbspq_snd1(SERVER_Q_INDEX, packet, packet_size);

  return(status);
}

int e_nbspq_snd1(int type, void *packet, size_t packet_size){
  /*
   * Send to one queue channel
   */
  int status = 0;
  int qerrno = 0;
  uint32_t packet_size_uint32;

  packet_size_uint32 = packet_size;
  status = nbspqt_snd(g.qtable, type, packet, packet_size_uint32, &qerrno);
  if(status == -1){
    q_log_err("Error sending product to qtable.", qerrno);
    return(-1);
  }

  nbspq_set_quota_status1(type, status);
  if(status == 1){
    /* 
     * Soft limit reached; will be reported periodically.
     */
    status = 0;
  } else if(status == 2) {
    /* 
     * Hard limit reached; will be reported periodically, but report
     * it here as well.
     */
    log_errx("Rejecting additions to queue[%d].", type);
    status = -1;
  }

  return(status);
}

int e_nbspq_rcv_filter(void **packet, size_t *packet_size){

  return(e_nbspq_rcv1(FILTER_Q_INDEX, packet, packet_size));
}

int e_nbspq_rcv_server(void **packet, size_t *packet_size){

  return(e_nbspq_rcv1(SERVER_Q_INDEX, packet, packet_size));
}

int e_nbspq_rcv1(int type, void **packet, size_t *packet_size){

  int status;
  int qerrno;
  uint32_t packet_size_uint32;

  packet_size_uint32 = *packet_size;
  status = nbspqt_rcv(g.qtable, type, packet, &packet_size_uint32,
		      g.servers_queue_read_timeout_ms, &qerrno);

  if(status == 0)
    *packet_size = packet_size_uint32;

  if(status == 1){
    /*
     *  log_info("qtable empty.",);
     */
    log_verbose(1, "queue[%d] empty.", type);
  }else if(status != 0){
    log_errx("Cannot receive element from qtable[%d].", type);
    q_log_err("Error from nbspqt_rcv.", qerrno);
  }
    
  return(status);
}

static void nbspq_set_quota_status1(int type, int status){

  struct queue_state_st *qstate;

  /*
   * Get the qstate pointer to the first channel and then to
   * the channel given by "type".
   */
  qstate = (struct queue_state_st*)g.qtable->appdata;
  qstate += type;

  qstate->current = status;
}

static void e_nbspq_quota1(int type){

  struct queue_state_st *qstate;

  /*
   * Get the qstate pointer to the first channel and then to
   * the channel given by "type".
   */
  qstate = (struct queue_state_st*)g.qtable->appdata;
  qstate += type;

  /*
   * Now check the limits, and report only if:
   * (1) the quota is exceeded.
   * (2) an overflow was restored.
   */
     
  if(qstate->current == 1){
    log_warnx("Soft size-limit reached for queue[%d]", type);
  }else if(qstate->current == 2){
      log_errx("Hard size-limit reached for queue[%d]", type);
  }else if(qstate->current != qstate->last){
    log_warnx("Size restored for queue[%d]", type);
  }
  qstate->last = qstate->current;
}

void e_nbspq_report_quota(void){

  int i;

  for(i = 0; i < g.queue_numchannels; ++i){
    e_nbspq_quota1(i);
  }
}

void e_nbspq_dbstats(void){

  nbsp_dbstats(g.dbenv, g.qdbstatslogfile);
}

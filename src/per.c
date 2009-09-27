/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <time.h>
#include "globals.h"
#include "signal.h"
#include "load.h"
#include "nbsp.h"
#include "nbspq.h"
#include "qstate.h"
#include "nbspmspoolbdb.h"
#include "stats.h"
#include "filters.h"
#include "server.h"
#include "exec.h"
#include "per.h"

#define PERIOD_HALF_MINUTE	30
#define PERIOD_MINUTE		60
#define PERIOD_5MINUTES		300
#define PERIOD_HOUR		3600
#define PERIOD_2HOURS		7200

/*
 * These are periodic events for which it does not matter exactly when they
 * are called.
 */
struct periodic_event_st {
  time_t period;
  time_t next;
  void (*proc)(void);
};

/* Events for which we must guarantee that they are called each minute. */
struct minutely_event_st {
  int last_minute;
  void (*proc)(void);
};

#define EVENT_REPT_PCTL_QUOTA	0
#define EVENT_REPT_QDB_QUOTA	1
#define EVENT_LOG_QSTATE	2
#define EVENT_UPDATE_NBSP_STATS	3
#define EVENT_SERVER_STATE	4
#define EVENT_FILTERSERVER_STATE 5
#define EVENT_LOADAVE_COND	6
#define EVENT_RTXDB_TRUNCATE	7
#define EVENT_MSPOOLBDB_DBSTATS 8

static struct periodic_event_st gevents [] = {
  {PERIOD_MINUTE, 0, rept_pctl_quota},
  {PERIOD_MINUTE, 0, e_nbspq_report_quota},
  {PERIOD_MINUTE, 0, log_qstate},
  {PERIOD_MINUTE, 0, nbspstats_update},
  {PERIOD_MINUTE, 0, set_server_state_flag},
  {PERIOD_MINUTE, 0, set_filterserver_state_flag},
  {PERIOD_HALF_MINUTE, 0, verify_load_ave_condition},
  {PERIOD_2HOURS, 0, set_rtxdb_truncate_flag},  
  {PERIOD_MINUTE, 0, nbsp_mspoolbdb_dbstats},
  {0, 0, NULL}
};

#define EVENT_SCHEDULER		0
static struct minutely_event_st gmevents [] = {
  {0, exec_scheduler},
  {0, NULL}
};

static int current_minute(time_t now);

void init_periodic(void){

  struct periodic_event_st *ev = &gevents[0];
  struct minutely_event_st *mev = &gmevents[0];
  time_t now;
  int minute_now;

  /*
   * If the default periods above are changed by the configuration file
   * we use that instead.
   */
  gevents[EVENT_REPT_PCTL_QUOTA].period = g.queue_quota_logperiod_secs;
  gevents[EVENT_REPT_QDB_QUOTA].period = g.queue_quota_logperiod_secs;
  gevents[EVENT_LOG_QSTATE].period = g.qstate_logperiod_secs;
  gevents[EVENT_UPDATE_NBSP_STATS].period = g.nbspstats_logperiod_secs;
  gevents[EVENT_SERVER_STATE].period = g.serverstate_logperiod_secs;
  gevents[EVENT_FILTERSERVER_STATE].period = g.serverstate_logperiod_secs;
  gevents[EVENT_LOADAVE_COND].period = g.loadave_checkperiod_secs;
  gevents[EVENT_RTXDB_TRUNCATE].period = g.rtxdb_truncate_minutes * 60;
  gevents[EVENT_MSPOOLBDB_DBSTATS].period = g.mspoolbdb_dbstats_logperiod_secs;

  now = time(NULL);
  minute_now = current_minute(now);

  while(ev->proc != NULL){
    ev->next = now + ev->period;
    ++ev;
  }

  while(mev->proc != NULL){
    mev->last_minute = minute_now;
    ++mev;
  }
}

void periodic(void){
  /*
   * This function is called from the main loop thread.
   *
   * Call first the functions that are scheduled to run at some
   * specified intervals. Then those that should be run each time
   * periodic() is called.
   */
  struct periodic_event_st *ev = &gevents[0];
  struct minutely_event_st *mev = &gmevents[0];
  time_t now;
  int minute_now;

  now = time(NULL);
  minute_now = current_minute(now);
  while(ev->proc != NULL){
    if(now >= ev->next){
      ev->next += ev->period;
      ev->proc();
    }
    ++ev;
  }

  while(mev->proc != NULL){
    if(minute_now != mev->last_minute){
      mev->last_minute = minute_now;
      mev->proc();
    }
    ++mev;
  }

  /*
   * This causes to check if the state fifo is opened for reading (on the
   * other end), and then open it for writing (on this end) to report the
   * state of the queue.
   */
  set_reopen_qstatefifo_flag();

  /*
   * This is the mechanism used to reload the filters. Just let
   * the filter thread know it should reload the filters.
   */
  if(get_hup_flag() != 0){
    set_reload_filters_flag();
    /*
     * set_reload_server_filters_flag();
     */
  }
}

static int current_minute(time_t now){

  struct tm tm, *tmp;
  time_t secs;
  int minute;

  secs = now;
  tmp = localtime_r(&secs, &tm);
  minute = tmp->tm_min;

  return(minute);
}

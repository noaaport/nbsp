/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include "config.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pthread.h>
#ifndef HAVE_DAEMON
#include "solaris.h"
#endif
#include "err.h"
#include "globals.h"
#include "defaults.h"
#include "util.h"
#include "server.h"
#include "pid.h"
#include "pw.h"
#include "qstate.h"
#include "npcast.h"
#include "filters.h"
#include "nbsp.h"
#include "reader.h"
#include "nbspq.h"
#include "rtxdb.h"
#include "nbspmspoolbdb.h"
#include "spooltype.h"
#include "efile.h"
#include "nbspre.h"
#include "dbenv.h"
#include "slave.h"
#include "httpd.h"
#include "conf.h"
#include "exec.h"
#include "init.h"

static int e_np_open(void);
static void e_np_close(void);
static void close_feeds(void);
static void close_queues(void);
static void close_mspool(void);

void init_globals(void){

  int i;

  /*
   * defaults
   */
  g.defconfigfile = CONFIGFILE;
  g.configfile = NULL;
  g.option_C = 0;
  g.option_F = 0;
  g.option_disable_filters = 0;
  g.option_disable_server = 0;
  g.option_disable_readers = 0;

  g.user = NBSP_USER;
  g.group = NBSP_GROUP;
  g.umask = DEFAULT_UMASK;
  g.multicastip = MULTICASTIP;
  g.multicastport = MULTICASTPORT;
  g.ifname = NBSP_IFNAME;		/* NULL => use default (kernel) */
  g.ifip = NBSP_IFIP;			/* NULL => use default (kernel) */
  g.udprcvsize = UDP_RCV_SIZE;
  g.startscript = START_SCRIPT;	/* to start/stop external programs (innd) */
  g.stopscript =  STOP_SCRIPT;
  g.scheduler = SCHEDULER;

  g.spooltype = SPOOLTYPE_DEFAULT;
  /* fs based spool */
  g.spooldir = NBSP_SPOOL_DIR;
  g.spooldb_slots = SPOOLDB_SLOTS;
  g.spooldb_fname = SPOOLDB_FNAME;

  /* bdb spool cache */
  g.cspoolbdb_dir = CSPOOLBDB_DIR;
  g.cspoolbdb_name = CSPOOLBDB_NAME;
  g.cspoolbdb_mode = CSPOOLBDB_DBMODE;
  g.cspoolbdb_dbcache_mb = CSPOOLBDB_DBCACHE_MB;
  g.cspoolbdb_ndb = CSPOOLBDB_NDB;
  g.cspoolbdb_nslots = CSPOOLBDB_NSLOTS;
  g.cspoolbdb_maxsize_per128 = CSPOOLBDB_MAXSIZE_PER128;

  /* memory bdb based spool */
  g.mspoolbdb_dbcache_mb = MSPOOLBDB_DBCACHE_MB;
  g.mspoolbdb_ndb = MSPOOLBDB_NDB;
  g.mspoolbdb_nslots = MSPOOLBDB_NSLOTS;
  g.mspoolbdb_maxsize_per128 = MSPOOLBDB_MAXSIZE_PER128;

  g.mspoolbdb_dbstats_logfile = MSPOOLBDB_DBSTATS_LOGFILE;
  g.mspoolbdb_dbstats_logperiod_s = MSPOOLBDB_DBSTATS_LOGPERIOD_SECS;

  g.filterdevdir = NBSP_FILTER_DEVDIR;
  g.filterserver_enable = FILTERSERVER_ENABLE;
  g.sysfilterlist = NBSP_SYSFILTERLIST;
  g.emwinfilter = NBSP_EMWINFILTER;
  g.emwinfilter_fifo = NBSP_EMWINFILTER_FIFO;
  g.emwinfilter_timeout_s = NBSP_EMWINFILTER_READ_SECS;
  g.emwinfilter_always = EMWINFILTER_ALWAYS;
  g.netfilter_enable = NBSP_NETFILTER_ENABLE;
  g.netfilter = NBSP_NETFILTER;
  g.httpd_enable = NBSP_HTTPD_ENABLE;
  g.httpd = NBSP_HTTPD;
  g.pidfile = NBSP_PIDFILE;
  g.statusfile = NBSP_STATUSFILE;
  g.missinglogfile = NBSP_MISSINGLOGFILE;
  g.rtxlogfile = NBSP_RTXLOGFILE;
  g.qstatelogfile = NBSP_QSTATELOGFILE;
  g.qdbstatslogfile = NBSP_QDBSTATSLOGFILE;
  g.qstatefifo = NBSP_QSTATEFIFO;
  g.serverstatefile = NBSP_SERVERSTATE_FILE;
  g.serveractivefile = NBSP_SERVERACTIVE_FILE;
  g.serverthreadsfile = NBSP_SERVERTHREADS_FILE;
  g.filterserver_statefile = NBSP_FILTERSERVER_STATEFILE;
  g.slavestatsfile = NBSP_SLAVESTATSFILE;
  g.qstatefifo_mode = NBSP_QSTATEFIFO_MODE;
  g.product_mode = FILE_PRODUCT_MODE;
  g.subdir_product_mode = SUBDIR_PRODUCT_MODE;
  g.saveframefmt = SAVE_FRAME_FMT;

  g.servertype = SERVER_TYPE;
  g.servername = SERVER_NAME;
  g.serverport = SERVER_PORT;
  g.server_listen_backlog = SERVER_LISTEN_BACKLOG;
  g.server_maxclients = SERVER_MAXCLIENTS;
  g.server_so_sndbuf = SERVER_SO_SNDBUF;
  g.server_clientid_timeout_s = SERVER_CLIENTID_TIMEOUT_SECS;

  g.nbspstats_logperiod_s = NBSP_STATS_LOG_PERIOD;
  g.qstate_logperiod_s = QSTATE_LOG_PERIOD;
  g.serverstate_logperiod_s = SERVERSTATE_LOG_PERIOD;
  g.serverthreads_logfreq = SERVERTHREADS_LOG_FREQ;
  g.broadcast_read_timeout_s = BROADCAST_READ_TIMEOUT_SECS;
  g.fifo_write_timeout_ms = FIFO_WRITE_TIMEOUT_MSECS;
  g.client_write_timeout_ms = CLIENT_WRITE_TIMEOUT_MSECS;
  g.client_write_timeout_retry = CLIENT_WRITE_TIMEOUT_RETRY;
  g.clientoptions = NULL;
  g.servers_queue_read_timeout_ms = SERVERS_QUEUE_READ_TIMEOUT_MSECS;
  g.sthreads_queue_read_timeout_ms = STHREADS_QUEUE_READ_TIMEOUT_MSECS;
  g.processor_pctl_read_timeout_ms = PROCESSOR_PCTL_READ_TIMEOUT_MSECS;

  g.memfile_blocksize = MEMFILE_BLOCKSIZE;
  g.memfile_minsize = MEMFILE_MINSIZE;

  g.dbhome = QUEUE_DBHOME;
  g.dbcache_mb = QUEUE_DBCACHE_MB;
  g.dbextent_size = QUEUE_DBEXTENT_SIZE;
  g.dbfile_mode = DBFILE_MODE;

  g.pctl_dbfname = PCTL_DBFNAME;
  g.pctlmf_dbfname = PCTLMF_DBFNAME;
  g.qdb_dbfname = QDB_DBFNAME;		/* filter and server */

  g.queue_maxsize_soft = QUEUE_MAXSIZE_SOFT;
  g.queue_maxsize_hard = QUEUE_MAXSIZE_HARD;
  g.queue_repquota_period_s = QUEUE_REPQUOTA_PERIOD_SECS;

  g.pctl_maxsize_soft = PCTL_MAXSIZE_SOFT;
  g.pctl_maxsize_hard = PCTL_MAXSIZE_HARD;
  g.pctl_maxmem_soft = PCTL_MAXMEM_SOFT_MB;
  g.pctl_maxmem_hard = PCTL_MAXMEM_HARD_MB;

  g.client_queue_maxsize_soft = CLIENT_QUEUE_MAXSIZE_SOFT;
  g.client_queue_maxsize_hard = CLIENT_QUEUE_MAXSIZE_HARD;
  g.client_queue_dbcache_mb = CLIENT_QUEUE_DBCACHE_MB;

  g.rtxdb_dbfname = NBSP_RTXDB_DBFNAME;
  g.rtxdb_slots = NBSP_RTXDB_SLOTS;
  g.rtxdb_truncate_minutes = NBSP_RTXDB_TRUNCATE_MINUTES;
  g.rtxdb_default_process = NBSP_RTXDB_DEFAULT_PROCESS;

  g.max_load_ave_hard = MAX_LOAD_AVE_HARD;
  g.max_load_ave_soft = MAX_LOAD_AVE_SOFT;
  g.max_load_ave_sleep_secs = MAX_LOAD_AVE_SLEEP_SECS;
  g.max_load_ave_period_secs = MAX_LOAD_AVE_PERIOD_SECS;
  g.max_load_rtx_index = MAX_LOAD_RTX_INDEX;

  g.feedmode = DEFAULT_FEEDMODE;
  g.masterservers = SLAVE_MASTERSERVERS;
  g.slave_read_timeout_s = SLAVE_READ_TIMEOUT_SECS;
  g.slave_read_timeout_retry = SLAVE_READ_TIMEOUT_RETRY;
  g.slave_reopen_timeout_s = SLAVE_REOPEN_TIMEOUT_SECS;
  g.slave_so_rcvbuf = SLAVE_SO_RCVBUF;
  g.slave_stats_logperiod_secs = SLAVE_STATS_LOGPERIOD_SECS;
  g.infifo = SLAVE_INPUT_FIFO;
  g.infifo_mode = SLAVE_INPUT_FIFO_MODE;
  g.infifo_grp = SLAVE_INPUT_FIFO_GRP;

  g.patt_accept = PATT_ACCEPT;
  g.filterq_patt_accept = FILTERQ_PATT_ACCEPT;
  g.serverq_patt_accept = SERVERQ_PATT_ACCEPT;
  g.nbs1_patt_accept = NBS1_PATT_ACCEPT;
  g.nbs2_patt_accept = NBS2_PATT_ACCEPT;
  g.emwin_patt_accept = EMWIN_PATT_ACCEPT;
  g.savez_patt_accept = SAVEZ_PATT_ACCEPT;
  g.rtxdb_patt_accept = RTXDB_PATT_ACCEPT;

  /*
   * variables
   */
  g.pctl = NULL;
  g.rtxdb = NULL;
  g.qtable = NULL;
  g.queue_numchannels = QUEUE_NUMCHANNELS;
  g.dbenv = NULL;
  g.spooldb = NULL;
  g.cspoolbdb = NULL;
  g.mspoolbdb = NULL;
  g.httpdfp = NULL;
  nbspstats_init();
  /* pthread_t server_thread_id */
  /* pthread_t filter_thread_id */
  /* pthread_t processor_thread_id */
  /* pthread_t reader_thread_id[] */
  /* pthread_t slave_thread_id */
  g.f_filter_thread_created = 0;
  g.f_server_thread_created = 0;
  g.f_processor_thread_created = 0;
  for(i = 0; i <= NPCAST_NUM_CHANNELS - 1; ++i){
    g.f_reader_thread_created[i] = 0;
  }
  g.f_slave_thread_created = 0;
  g.np_regex = NULL;
  g.filterq_regex = NULL;
  g.serverq_regex = NULL;
  g.nbs1_regex = NULL;
  g.nbs2_regex = NULL;
  g.emwin_regex = NULL;
  g.savez_regex = NULL;
  g.rtxdb_regex = NULL;
  g.ct = NULL;
  g.server_fd = -1;
  g.fifo_fd = -1;
  g.slavet = NULL;
  g.qstatefifo_fd = -1;
  g.qstatefifo_buffer = NULL;
  g.qstatefifo_buffer_size = 0;
  g.f_nbs1server_enabled = 0;
  g.f_nbs2server_enabled = 0;
  g.f_emwinserver_enabled = 0;
  g.f_lock = 0;
  g.f_debug = 0;
  g.f_ndaemon = 0;
  g.f_verbose = 0;
  g.f_max_load_ave = 0;
  g.f_max_load_rtx = 0;
}

void cleanup(void){

  kill_httpd_server();
  kill_reader_threads();
  kill_filter_thread();
  kill_slave_threads();
  kill_server_thread();
  kill_processor_thread();

  /*
   * The feeds (noaaport or slave) are "opened" in main (init_feeds)
   * and therefore they must be closed here. The processor, server and filter
   * threads initialize themselves, and therefore close themselves by calling
   *
   * close_nbsproc();
   * close_server();
   * close_filter_server();
   */
  close_feeds();
  close_queues();
  close_mspool();

  release_confoptions();
  free_nbsp_regex();

  cleanup_files();

  log_info("Stopped.");
  (void)exec_stopscript();
}

void cleanup_files(void){

  kill_qstate_fifo();
  if(g.f_lock == 1){
    if(remove_pidfile(g.pidfile) != 0)
      log_err("Could not remove pidfile.");

    g.f_lock = 0;
  }
}
  
int init_daemon(void){

  int status = 0;

  if(g.option_F == 0)
    status = daemon(0, 0);

  if(status != 0)
    return(status);

  umask(g.umask);
  openlog(SYSLOG_IDENT, SYSLOG_OPTS, SYSLOG_FACILITY);
  set_log_daemon();

  return(status);
}

int init_lock(void){
  /*
   * This has to be done after daemon() so that the lock file  contains the
   * daemon's pid, not the starting program's.
   */
  int status = 0;

  if(create_pidfile(g.pidfile) != 0){
    log_err2("Could not create", g.pidfile);
    status = 1;
  }else
    g.f_lock = 1;

  return(status);
}

int init_directories(void){

  int status = 0;

  status = e_dir_exists(g.spooldir);
  if(status == 0)
    status = e_dir_exists(g.dbhome);

  return(status);
}

int drop_privs(void){
  /*
   * Change the group first.
   */
  int status = 0;

  if(valid_str(g.group)){
    status = change_group(g.group);
    if(status != 0)
      log_err2("Could not change to group", g.group);
  }

  if((status == 0) && valid_str(g.user)){
    status = change_user(g.user);
    if(status != 0)
      log_err2("Could not change to user", g.user);
  }

  return(status);
}

static int e_np_open(void){

  int status;

  status = np_open(g.multicastip, g.multicastport, g.ifname, g.ifip,
		   g.udprcvsize);

  if(status == -1)
    log_err("Cannot join multicast.");
  else if(status != 0)
    log_errx("Multicast channel configuration error.");
  else
    log_info("Opened noaaport multicast channels.");

  return(status);
}

static void e_np_close(void){

  np_close();

  log_info("Closed noaaport multicast channels.");
}

int init_queues(void){
  /*
   * Initialize all the queues before starting any processing thread.
   * This fucnction must be called after initializing the feeds (see
   * init_feeds() below).
   */
  int status = 0;

  log_info("Initializing db queues.");

  /*
   * Open the db environment before any of the db queues are opened.
   */
  status = nbsp_open_dbenv();
  if(status == 0)
    log_info("dbenv opened.");

  /*
   * pctl and received files db.
   */
  if(status == 0)
    status = init_pctl();	/* pctl */

  if(status == 0)
    log_info("pctldb initialized.");

  if(status == 0)
    status = e_nbsp_rtxdb_open();	/* rtxdb */

  if(status == 0)
    log_info("rtx db initialized.");

  /*
   * The filter and server queues, shared between the network and filter
   * servers and the processor (libqdb).
   */
  if(status == 0)  
    status = init_nbspq();

  if(status == 0)
    log_info("nbspq initialized.");

  if(status == 0)
    log_info("Initialized db queues.");

  return(status);
}

static void close_queues(void){
  /*
   * The reverse of the above.
   */

  kill_pctl();		/* pctl (db) */
  e_nbsp_rtxdb_close();	/* rtx (db) */
  kill_nbspq();		/* libqdb (db) */
  nbsp_close_dbenv();   /* Close dbenv after all db queues are closed. */
}

int init_feeds(void){
  /*
   * This function creates the slave table and therefore, if there are
   * slave nbs readers, it determines how many pctl readers there are.
   * The number of pctl readers is required for initializing the pctl
   * (see init_pctl() in nbsp.c) and therefore this function init_feed()
   * must be called before the the function that initializes the pctl
   * init_queues().
   */
  int status = 0;

  if(feedmode_noaaport_enabled())
    status = e_np_open();

  if(status == 0){
    /*
     * The slave table must initialized if there are slaves either
     * network slaves or input fifo(s).
     */
    if(feedmode_slave_enabled())
      status = init_slavet();
  }

  return(status);
}

static void close_feeds(void){

  e_np_close();		/* noaaport connections */
  cleanup_slavet();	/* net slave table */
}

int spawn_feeds(void){

  int status = 0;
    
  if(feedmode_noaaport_enabled())
    status = spawn_readers();

  if(status == 0){
    if(feedmode_slave_enabled())
      status = spawn_slave_threads();
  }

  return(status);
}

int spawn_processor(void){

  int status = 0;

  /*
   * The fpath (nbs2) and infeed slaves do not need the processor;
   * only the noaaport and nbs1 readers need it. By the time this
   * function is called, the slave table must have been initialized
   * so that the number of nbs1 readers has been determined.
   */
  if(feedmode_noaaport_enabled() || feedmode_slave_nbs1_enabled())
    status = spawn_nbsproc();

  return(status);
}

int init_mspool(void){
  /*
   * If the mspoolbdb is enabled they are used by the processor
   * and also by the filter server to extract the metadata to send
   * to the filters. Therefore it must be initialized before spawning
   * those threads, and cleared after those threads terminate.
   */
  int status = 0;

  if(spooltype_cspool()){
    status = nbsp_cspoolbdb_create();
  }else if(spooltype_mspool()){
    status = nbsp_mspoolbdb_create();
  }

  return(status);
}

static void close_mspool(void){

  (void)nbsp_cspoolbdb_destroy();
  (void)nbsp_mspoolbdb_destroy();
}

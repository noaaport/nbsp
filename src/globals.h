/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>		/* FILE */
#include <fcntl.h>		/* mode_t */
#include <pthread.h>
#include <db.h>		
#include "libqdb/qdb.h"		/* nbspqtable_t */
#include "libspoolbdb/mspoolbdb.h"
#include "ure.h"
#include "pctl.h"
#include "stats.h"
#include "slavet.h"
#include "defaults.h"

struct nbsp_globals {
  char *defconfigfile;  /* the default */
  char *configfile;     /* user-specified (via -c) */
  int   option_C;       /* [-C] just print configuration and exit */
  int   option_F;	/* [-F] don't call daemon() */
  int   option_disable_filters; /* [-f] no filter thread */
  int   option_disable_server; /* [-s] no server thread */
  int   option_disable_readers; /* [-q] no readers; just process queues */
  /*
   * configurable options 
   */
  char *user;
  char *group;
  mode_t umask;
  char *multicastip;
  char *multicastport;
  char *ifname;			/* interface name (xl0, fxp0, ..) to use */
  char *ifip;			/* interface ip (ip has priority) */
  int udprcvsize;		/* size of udp buffer */
  char *startscript;		/* run just before becoming daemon */
  char *stopscript;		/* run just before exiting */
  char *scheduler;
  int spooltype;		/* fs based or memory bdb */
  /* fs spool */
  char *spooldir;
  unsigned int spooldb_slots;	/* num of files kept in the spool directory */
  char *spooldb_fname;		/* file to save (load) the spool slots state */
  /* bdb spool cache */
  char *cspoolbdb_dir;		/* bdb spool cache dir */
  char *cspoolbdb_name;		/* name of the cspool db */
  mode_t cspoolbdb_mode;
  unsigned int cspoolbdb_dbcache_mb; /* for each db */
  unsigned int cspoolbdb_ndb;	/* number of databases */
  int cspoolbdb_nslots;		/* buffer slots for reading (server) */
  unsigned int cspoolbdb_maxsize_per128; /* maxsize as a fraction of 128 */
  /* memory bdb spool */
  unsigned int mspoolbdb_dbcache_mb; /* for each db */
  unsigned int mspoolbdb_ndb;	/* number of databases */
  int mspoolbdb_nslots;		/* buffer slots for reading (server) */
  unsigned int mspoolbdb_maxsize_per128; /* maxsize as a fraction of 128 */
  char *mspoolbdb_dbstats_logfile;   /* file to write the mspoolbdb dbstats */
  time_t mspoolbdb_dbstats_logperiod_secs;
  char *filterdevdir;		/* directory for external filters and fifos */
  int filterserver_enable;
  char *sysfilterlist;		/* list of filters distributed with program */
  char *emwinfilter;
  char *emwinfilter_fifo;
  int emwinfilter_read_timeout_secs;	/* reading timeout (in secs) */
  int emwinfilter_always;
  int netfilter_enable;
  char *netfilter;
  int httpd_enable;
  char *httpd;			/* tclhttpd script */
  char *pidfile;
  char *statusfile;		/* statistics summary */
  char *missinglogfile;		/* missed products */
  char *rtxlogfile;		/* retransmitted (successfully) products */
  char *qstatelogfile;		/* state of the queues (periodic) */
  char *qdbstatslogfile;	/* file to write the qdb dbstats */
  char *qstatefifo;		/* state of the queues (continuous) */
  char *serverstatefile;	/* server client connections */
  char *serveractivefile;	/* server active client connections */
  char *serverthreadsfile;	/* server threads stats */
  char *filterserver_statefile;
  char *slavestatsfile;		/* the slave threads stats logfile */
  mode_t qstatefifo_mode;
  mode_t product_mode;
  mode_t subdir_product_mode;	/* mode of subdirs where produtcs are saved */
  int saveframefmt;           /* how the (ccb, wmo) header is saved */
  int servertype;		/* protocol of the spawned server */
  char *servername;
  char *serverport;
  int server_listen_backlog;
  int server_maxclients;
  int server_so_sndbuf;
  int server_clientid_timeout_secs; /* net client limit to identify itself */
  time_t nbspstats_logperiod_secs;
  time_t qstate_logperiod_secs;	/* how often to log the state of the queues */
  time_t serverstate_logperiod_secs; /* how often to log the server state */
  int serverthreads_logperiod_count; /* how often to log the server threads */
  int broadcast_read_timeout_secs;  /* readers timeout waiting for broadcast */
  int fifo_write_timeout_ms;	   /* filter server timeout writing to fifos */
  int client_write_timeout_ms;  /* net server timeout writing to clients */
  int client_write_timeout_retry;
  int client_reconnect_wait_sleep_secs;
  int client_reconnect_wait_sleep_retry;
  char *clientoptions;		/* per-host client options */
  int servers_queue_read_timeout_ms; /* servers timeout reading from queues */
  int sthreads_queue_read_timeout_ms; /* same for servers' threads */
  int processor_pctl_read_timeout_ms; /* processor thread waiting for pctl */
  int memfile_blocksize;	/* for the mfile routines */
  int memfile_minsize;		/* same as above */
  char *dbhome;			/* home dir for db queues (for dbenv) */
  int dbcache_mb;		/* memory cache size for dbenv */
  int dbextent_size;		/* # of pages in each extent file */
  mode_t dbfile_mode;		/* mode for all the db files created */
  /* file names for the db queue files */
  char *pctl_dbfname;		/* dbfname for pctl */
  char *pctlmf_dbfname;		/* for the pctl mf db */
  char *qdb_dbfname;		/* queue of filter and server */
  /* db configuration */
  int queue_maxsize_soft;	/* maximum size (soft) of servers queues */
  int queue_maxsize_hard;	/* same but hard */
  int queue_quota_logperiod_secs;
  int pctl_maxsize_soft;	/* maximum sizes of pctl queue */
  int pctl_maxsize_hard;
  int pctl_maxmem_soft;		/* mem limits for pctlmfdb queue (in mb) */
  int pctl_maxmem_hard;
  int client_queue_maxsize_soft; /* same for network client queues */
  int client_queue_maxsize_hard; /* same for network client queues */
  int client_queue_dbcache_mb;
  char *rtxdb_dbfname;		/* processor db of received files */
  unsigned int rtxdb_slots;	/* number of slots in rtxdb */
  unsigned int rtxdb_truncate_minutes;
  int rtxdb_default_process;	/* default action if rtxdb is disabled */
  int loadave_max_hard;		/* when readers will be throttled */
  int loadave_max_soft;
  int loadave_max_sleep_secs;	/* for how long */
  int loadave_checkperiod_secs;	/* how often to check */
  int loadave_max_rtx_index;	/* max retrans. accepted in high load cond. */
  int feedmode;			/* master, net slave, input fifo */
  char *masterservers;		/* master hosts list for net slave mode */
  int slave_read_timeout_secs;	/* timeout when slave reads from master */
  int slave_read_timeout_retry;
  int slave_reopen_timeout_secs; /* sleep secs before reopening connection */
  int slave_so_rcvbuf;
  int slave_stats_logperiod_secs;
  int slave_reject_duplicates;
  char *infifo;			/* fifo name for input feed mode */
  mode_t infifo_mode;
  char *infifo_grp;
  /* Regexps for file name pattern matching */
  char *patt_accept;		
  char *filterq_patt_accept;
  char *serverq_patt_accept;
  char *nbs1_patt_accept;
  char *nbs2_patt_accept;
  char *emwin_patt_accept;
  char *savez_patt_accept;	/* files that are saved compressed */
  char *rtxdb_patt_accept;
  /*
   * internal variables
   */
  struct pctl_st *pctl;		/* readers and processor pctl (db) */
  struct nbsp_rtxdb_st *rtxdb;	/* received files db (rtxdb.h) */
/* Number of channels in the table queue (filter and server) */
#define QUEUE_NUMCHANNELS	2
#define FILTER_Q_INDEX		0
#define SERVER_Q_INDEX		1
  int queue_numchannels;	
  nbspqtable_t  *qtable;	/* server and filter db queue's */
  DB_ENV *dbenv;		/* environment for all db queues */
  struct spooldb_st *spooldb;	/* the spool directory bookeeping array */
  struct mspoolbdb_st *cspoolbdb; /* bsd spool cache */
  struct mspoolbdb_st *mspoolbdb; /* in-memory bdb spool */
  FILE *httpdfp;
  struct nbsp_stats_st nbspstats;
  pthread_t filter_thread_id;
  pthread_t server_thread_id;
  pthread_t processor_thread_id;
  pthread_t reader_thread_id[NPCAST_NUM_CHANNELS];
  pthread_t slave_thread_id;
  int f_filter_thread_created;
  int f_server_thread_created;
  int f_processor_thread_created;
  int f_reader_thread_created[NPCAST_NUM_CHANNELS];
  int f_slave_thread_created;
  struct uwildregex_st *np_regex;
  struct uwildregex_st *filterq_regex;
  struct uwildregex_st *serverq_regex;
  struct uwildregex_st *nbs1_regex;
  struct uwildregex_st *nbs2_regex;
  struct uwildregex_st *emwin_regex;
  struct uwildregex_st *savez_regex;   /* files that are saved compressed */
  struct uwildregex_st *rtxdb_regex;
  struct conn_table_st *ct;	/* libconn2 table */
  int server_fd;		/* fd to accept network connections */
  int fifo_fd;			/* fd to use for input fifo mode */
  struct slave_table_st *slavet; /* masterservers table for net slave mode */
  int qstatefifo_fd;		/* fifo for sending internal queues state */
  void *qstatefifo_buffer;
  int qstatefifo_buffer_size;
  int f_nbs1server_enabled;
  int f_nbs2server_enabled;
  int f_emwinserver_enabled;
  int f_lock;			/* pid created? */
  int f_debug;			/* debug mode set */
  int f_ndaemon;		/* don't become daemon */
  int f_verbose;		/* mostly for debuging */
  int f_loadave_max;		/* flag set by calling getloadavg */
  int f_loadave_max_rtx;	/* flag set by calling getrtxindex and load */
} g;


#endif

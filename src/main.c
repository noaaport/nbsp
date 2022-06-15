/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <tcl.h>	/* Tcl_FindExecutable to init tcl */
#include "err.h"
#include "sbn.h"
#include "unz.h"
#include "pctl.h"
#include "mcast.h"
#include "reader.h"
#include "nbsp.h"
#include "filters.h"
#include "nbspre.h"
#include "init.h"
#include "nbspq.h"
#include "per.h"
#include "qstate.h"
#include "signal.h"
#include "server.h"
#include "httpd.h"
#include "exec.h"
#define NBSP_GLOBALS_DEF
#include "globals.h"
#include "defaults.h"
#include "conf.h"

static int parse_args(int argc, char **argv);
static int loop(void);

int main(int argc, char **argv){

  int status = 0;

  init_globals();
  atexit(cleanup);

  /*
   * PROBLEM
   * We used to call init_signals() only after init_daemon(). But in
   * that case, when started with -F or -D -D, the signals are
   * not caught in Linunx and OSX (they are caught in FreeBSD). nbspd and
   * npemwind die, but leave the pid file and the web server.
   * [It seems that the signals are not blocked in the main thread as
   * the code in signal.c should ensure.]
   * Adding this call here
   *
   * status = init_signals();
   *
   * makes OSX and Linux respond well when the daemon is run in the foreground.
   * If the call is made after the tcl configure(), the problem repeats;
   * it has to be before the configure() function.
   *
   * The problem is that in FreeBSD-7.1, when init_signals() is called here,
   * then no threads are spawned afterwards.
   *
   * The solution was to split init_signals() in two parts, one that
   * block the signals and the other spawns the thread. I don't fully
   * understand what in tcl is causing this (Fri Mar 13 11:43:09 AST 2009).
   */
  status = init_signals_block();

  if(status == 0){
    /*
     * This will configure it with the default configuration file,
     * if it exists.
     *
     * First initialize the tcl library once and for all. It was not
     * necessary to call this in unix, but cygwin needs it or EvalFile
     * seg faults.
     */
    Tcl_FindExecutable(argv[0]);
    status = configure();
  }

  if(status == 0)
    status = parse_args(argc, argv);

  if(status == 0){
    if(g.configfile != NULL){
      /*
       * This will reconfigure it with the user-supplied config file
       */
      status = configure();
    }
  }

  /*
   * if [-C] was given, print the configuration and exit.
   */
  if(status == 0){
    if(g.option_C == 1){
      print_confoptions();
      return(0);
    }
  }

  if(status == 0)
    status = validate_configuration();

  /*
   * user, group and home are configurable so this must be done after reading
   * configuration options.
   */
  if(status == 0)
    status = drop_privs();

  if(status == 0)
    status = init_nbsp_regex();

  if(status == 0)
    status = init_directories();

  /*
   * The last configuration step, just before becoming a daemon.
   */
  if(status == 0)
    status = exec_startscript();

  if((status == 0) && (g.f_ndaemon == 0))
    status = init_daemon();

  set_log_verbose(g.f_verbose);
  set_log_debug(g.f_debug);

  if(status == 0)
    status = init_signals_thread();

  /*
   * This has to be done after daemon() so that the lock file contains the
   * daemon's pid, not the starting program's.
   */
  if(status == 0)
    status = init_lock();

  if(status == 0)
    status = init_qstate_fifo();

  /*
   * The feeds must be initialized before the queues (see init.c).
   */
  if(status == 0)
    status = init_feeds();

  /*
   * Initialize all shared spools and queues before any threads are started.
   */
  if(status == 0)
    status = init_mspool();

  if(status == 0)
    status = init_queues();

  if(status == 0)
    init_periodic();

  /*
   * Launch all the threads. Leave the feed thread for last.
   */
  if(status == 0)
    status = spawn_processor();

  if((status == 0) && (g.option_disable_filters == 0)){
    if(g.filterserver_enable > 0)
      status = spawn_filter_server();
  }

  if((status == 0) && (g.option_disable_server == 0)){
    /*
     * This is the nbs1, nbs2, emwin network server thread.
     */
    if(g.servertype != BUILTIN_SERVER_NONE)
      status = spawn_server();
  }

  if(status == 0){
    if(g.httpd_enable > 0)
      status = spawn_httpd_server();
  }

  if((status == 0) && (g.option_disable_readers == 0))
    status = spawn_feeds();

  /*
   * If there are initialization errors, ask all threads to quit.
   */
  if(status != 0)
    set_quit_flag();

  while(get_quit_flag() == 0){
    status = loop();
  }

  if(status != 0)
    status = EXIT_FAILURE;

  return(status);
}

static int loop(void){

  int status = 0;

  /*
   * This is the main loop, which we use for monitoring or periodic
   * activities outside of all threads.
   */
  sleep(MAINLOOP_SLEEP_SECS);
  periodic();

  return(status);
}

static int parse_args(int argc, char **argv){

  int status = 0;
  int c;
  int conflict_frs = 0;
#ifdef DEBUG
  char *optstr = "c:frsCDFV";
  char *usage = "nbspd [-c configfile] [-frs] [-C] [-D] [-D] [-F] "
    "[-V] [-V] [-V]";
#else
  char *optstr = "c:frsCF";
  char *usage = "nbspd [-c configfile] [-frs] [-C] [-F]";
#endif

  while( (status == 0) && ((c = getopt(argc,argv,optstr)) != -1) ){
    switch(c){
    case 'c':
      g.configfile = optarg;
      break;
    case 'f':
      g.option_disable_filters = 1;
      ++conflict_frs;
      break;
    case 'r':
      g.option_disable_readers = 1;
      ++conflict_frs;
      break;
    case 's':
      g.option_disable_server = 1;
      ++conflict_frs;
      break;
    case 'C':
      g.option_C = 1;
      break;
    case 'F':
      g.option_F = 1;	/* daemon() not called. Added for launchd in OSX */
      break;
#ifdef DEBUG
    case 'D':
      ++g.f_debug;
      if(g.f_debug == 2){
	g.f_ndaemon = 1;
      }
      break;
    case 'V':
      ++g.f_verbose;
      break;
#endif
    default:
      status = 1;
      errx(1, "%s\n", usage);
      break;
    }
  }

  /*
   * Check conflicts.
   */
  if(conflict_frs >= 3){
    status = 1;
    errx(1, "%s\n", "Options frs specified simultaneously.");
  }

  return(status);
}

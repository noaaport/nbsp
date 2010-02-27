/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include "config.h"
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "globals.h"	/* g.f_debug */
#include "err.h"
#include "signal.h"

static int f_quit = 0;
static int f_hup = 0;
static int f_alarm = 0;
static int f_dbpanic = 0;	/* set by dbpanic */
static pthread_t signal_thread_id;
static pthread_mutex_t signal_mutex = PTHREAD_MUTEX_INITIALIZER;
static sigset_t signal_set;

static void *signal_thread(void *arg);
static void signal_handler(int sig);

static void set_hup_flag(void);
static void set_alarm_flag(void);
static void default_signals(void);

static void default_signals(void){

  /*
   * Make sure to restablish the default action. For some reason,
   * SIGHUP was being ignored (at least in FreeBSD) when the program
   * is started from the boot process.
   */
  signal(SIGALRM, SIG_DFL);
  signal(SIGHUP, SIG_DFL);
  signal(SIGPIPE, SIG_DFL);
  signal(SIGINT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  signal(SIGQUIT, SIG_DFL);
  signal(SIGABRT, SIG_DFL);
}

int init_signals(void){
  /*
   * Ref: Butenhof, p. 227-230.
   *
   * For the reasons mentioned in main.c, I split this function in the
   * two parts
   *
   * init_signals_block
   * init_signals_thread
   *
   * and these are the ones that are being used.
   */
  int status = 0;
  pthread_attr_t attr;

  default_signals();

  /*
   * They will be blocked in all threads except the one calling sigwait.
   */
  sigemptyset(&signal_set);
  sigaddset(&signal_set, SIGALRM);
  sigaddset(&signal_set, SIGHUP);
  sigaddset(&signal_set, SIGPIPE);
  sigaddset(&signal_set, SIGINT);
  sigaddset(&signal_set, SIGTERM);
  sigaddset(&signal_set, SIGQUIT);
  sigaddset(&signal_set, SIGABRT);

  status = pthread_sigmask(SIG_BLOCK, &signal_set, NULL);

  if(status == 0)
    status = pthread_attr_init(&attr);

  if(status == 0)
    status = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  if(status == 0)
    status = pthread_create(&signal_thread_id, &attr, signal_thread, NULL);

  if(status != 0)
    log_errx("Could not create signal thread: error %d.", status);

  return(status);
}

int init_signals_block(void){
  /*
   * Ref: Butenhof, p. 227-230.
   */
  int status = 0;

  default_signals();

  /*
   * They will be blocked in all threads except the one calling sigwait.
   */
  sigemptyset(&signal_set);
  sigaddset(&signal_set, SIGALRM);
  sigaddset(&signal_set, SIGHUP);
  sigaddset(&signal_set, SIGPIPE);
  sigaddset(&signal_set, SIGINT);
  sigaddset(&signal_set, SIGTERM);
  sigaddset(&signal_set, SIGQUIT);
  sigaddset(&signal_set, SIGABRT);

  status = pthread_sigmask(SIG_BLOCK, &signal_set, NULL);
  return(status);
}

int init_signals_thread(void){
  /*
   * Ref: Butenhof, p. 227-230.
   */
  int status = 0;
  pthread_attr_t attr;

  status = pthread_attr_init(&attr);
    
  if(status == 0)
    status = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  if(status == 0)
    status = pthread_create(&signal_thread_id, &attr, signal_thread, NULL);

  if(status != 0)
    log_errx("Could not create signal thread: error %d.", status);

  return(status);
}

static void *signal_thread(void *arg __attribute__((unused))){

  int status = 0;
  int signo;

  while(get_quit_flag() == 0){
    status = sigwait(&signal_set, &signo);
    if(status == 0)
      signal_handler(signo);
    else
      log_err("signal-thread error.");
  }

  return(NULL);
}

static void signal_handler(int sig){

  switch(sig){
  case SIGALRM:
    set_alarm_flag();
    log_info("Received alarm signal.");
    return;		/* don't exit */
    break;
  case SIGHUP:
    log_info("Received hangup signal.");
    set_hup_flag();
    return;		/* don't exit */
    break;
  case SIGPIPE:
    log_info("Client or pipe disconnected (SIGPIPE).");
    return;		/* don't exit */
    break;
  case SIGINT:
    log_info("Received interrupt signal.");
    break;
  case SIGTERM:
    log_info("Received terminate signal.");
    break;
  case SIGQUIT:
    log_info("Received quit signal.");
    break;
  case SIGABRT:
    log_info("Received abort signal.");
    if(g.f_debug){
      /*
       * Don't exit so that it dumps core.
       */
      ;
    }else
      exit(1);
    break;
  default:
    log_info("Received signal %d.", sig);
    return;
    break;
  }

  set_quit_flag();
}

int get_quit_flag(void){
  /*
   * Since the flag is set only once, and only by the signal thread,
   * while the other threads just read it, we will not protect it
   * with the mutex since strict syncrhonization is not required,
   * and on the other hand we avoid the overhead since this function
   * is called very frequently.
   */
  int flag;

  /*  pthread_mutex_lock(&signal_mutex); */

  flag = f_quit;

  /*  pthread_mutex_unlock(&signal_mutex); */

  return(flag);
}

int get_hup_flag(void){

  int flag;

  pthread_mutex_lock(&signal_mutex);
  flag = f_hup;
  f_hup = 0;
  pthread_mutex_unlock(&signal_mutex);

  return(flag);
}

int get_alarm_flag(void){

  int flag;

  pthread_mutex_lock(&signal_mutex);
  flag = f_alarm;
  f_alarm = 0;
  pthread_mutex_unlock(&signal_mutex);

  return(flag);
}

int get_dbpanic_flag(void){
  /*
   * This function is called only by the main thread, and only just before
   * stoping, and therefore it does not need to be mutex protected.
   */
  int flag;

  /*  pthread_mutex_lock(&signal_mutex); */

  flag = f_dbpanic;

  /*  pthread_mutex_unlock(&signal_mutex); */

  return(flag);
}

static void set_hup_flag(void){

  pthread_mutex_lock(&signal_mutex);
  f_hup = 1;
  pthread_mutex_unlock(&signal_mutex);
}

static void set_alarm_flag(void){

  pthread_mutex_lock(&signal_mutex);
  f_alarm = 1;
  pthread_mutex_unlock(&signal_mutex);
}

void set_quit_flag(void){
  /*
   * See the note in get_quit_flag.
   */

  /*  pthread_mutex_lock(&signal_mutex); */

  f_quit = 1;

  /*  pthread_mutex_unlock(&signal_mutex); */
}

void set_dbpanic_flag(void){
  /*
   * See the note in get_dbpanic_flag.
   */

  /*  pthread_mutex_lock(&signal_mutex); */

  f_dbpanic = 1;

  /*  pthread_mutex_unlock(&signal_mutex); */
}

/*
 * Copyright (c) 2002-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef LIBCONTH_CLIENT_OPTIONS
#define LIBCONTH_CLIENT_OPTIONS

struct client_options_st {
  int nonblock;
  int cloexec;
  int write_timeout_ms;
  int write_timeout_retry;
  int reconnect_wait_sleep_secs;
  int reconnect_wait_sleep_retry;
  int queue_read_timeout_ms;
};

#endif

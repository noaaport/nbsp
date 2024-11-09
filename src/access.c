/*
 * Copyright (c) 2004-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include "config.h"

#include <sys/types.h>
#include <syslog.h>
#include <stddef.h>
#ifdef HAVE_TCPWRAPPERS
#include <tcpd.h>
#endif
#include "defaults.h"
#include "err.h"
#include "access.h"

#ifdef HAVE_TCPWRAPPERS

#ifndef LIBWRAP_ALLOW_FACILITY
# define LIBWRAP_ALLOW_FACILITY LOG_AUTH
#endif
#ifndef LIBWRAP_ALLOW_SEVERITY
# define LIBWRAP_ALLOW_SEVERITY LOG_INFO
#endif
#ifndef LIBWRAP_DENY_FACILITY
# define LIBWRAP_DENY_FACILITY LOG_AUTH
#endif
#ifndef LIBWRAP_DENY_SEVERITY
# define LIBWRAP_DENY_SEVERITY LOG_WARNING
#endif

int deny_severity, allow_severity;

int client_allow_nconn(int client_fd, char *ip, char *name){
  /*
   * This is the function that we pass to libconn library to
   * use tcp_wrappers.
   */
  struct request_info request;
  int allow;
  char *nameorip = NULL;

  /*
   * Not being used at the moment, but anyway OpenBSD needs them defined
   * for linking.
   */
  deny_severity = LIBWRAP_DENY_FACILITY|LIBWRAP_DENY_SEVERITY;
  allow_severity = LIBWRAP_ALLOW_FACILITY|LIBWRAP_ALLOW_SEVERITY;

  request_init(&request,
               RQ_DAEMON, SYSLOG_IDENT,
               RQ_FILE, client_fd,
               0);
  fromhost(&request);

  allow = hosts_access(&request);

  if(name != NULL)
    nameorip = name;
  else if(ip != NULL)
    nameorip = ip;

  if(nameorip != NULL){
    if(allow == 0)
      log_info("Connection from %s denied in hosts.allow", nameorip);
    else
      log_info("Connection from %s.", nameorip);
  }

  return(allow);
}

#else

int client_allow_nconn(__attribute__((unused)) int client_fd,
			char *ip, char *name){
  /*
   * This is the function that we pass to libconn library in place
   * of the original one that was used with tcp_wrappers.
   */
  int allow = 1;
  char *nameorip = NULL;

  if(name != NULL)
    nameorip = name;
  else if(ip != NULL)
    nameorip = ip;

  if(nameorip != NULL){
    if(allow == 0)
      log_info("Connection from %s denied", nameorip);
    else
      log_info("Connection from %s.", nameorip);
  }

  return(allow);
}

#endif

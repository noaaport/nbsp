/*
 * Copyright (c) 2002-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef LIBCONNTH_TCPSOCK_H
#define LIBCONNTH_TCPSOCK_H

#include <sys/types.h>
#include <sys/socket.h>
#include "client_options.h"

int tcp_server_open_conn(const char *host, const char *serv,
			 int backlog, int so_sndbuf, int so_rcvbuf,
			 socklen_t *addrlenp, int *gai_code);
int tcp_server_accept_conn(int s, struct client_options_st *clientopts,
			   struct sockaddr *addr, socklen_t *addrlenp);
int tcp_client_open_conn(const char *host, const char *serv,
			 int so_sndbuf, int so_rcvbuf, int *gai_code);
int tcp_client_open_conn_timed(const char *host, const char *serv, int nsecs,
			       int so_sndbuf, int so_rcvbuf, int *gai_code);

char *get_peer_ip(int cfd);
char *get_peer_name(int cfd);

#endif

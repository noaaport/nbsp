/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id: mcast.h,v 1.10 2007/10/10 02:20:18 nieves Exp $
 */
#ifndef MCAST_H
#define MCAST_H

#include <sys/socket.h>

int mcast_rcv(char *host_name, char *service_port, char *ifname, char *ifip,
	      int udprcvsize, void **sa_ptr, socklen_t *sa_len, int *gai_code);
int mcast_snd(char *host_name, char *service_port, char *ifname, char *ifip,
	      int ttl, void **sa_ptr, socklen_t *sa_len, int *gai_code);
const char *sock_ntop(struct sockaddr *sa);
int udp_client(char *host, char *service, 
	       void **sa_ptr, socklen_t *sa_len, int *gai_code);
int udp_server(char *host, char *service, socklen_t *sa_len, int *gai_code);

int mcast_rcv_default(char *host_name, char *service_port,
                      void **sa_ptr, socklen_t *sa_len, int *gai_code);
int mcast_snd_default(char *host_name, char *service_port, int ttl,
                      void **sa_ptr, socklen_t *sa_len, int *gai_code);

#endif

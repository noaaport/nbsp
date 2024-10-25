/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id: mcast.h,v 1.4 2005/12/08 22:01:51 nieves Exp $
 */
#ifndef MCAST_H
#define MCAST_H

#include <sys/socket.h>

int mcast_rcv(char *host_name, char *service_port, char *ifname, char *ifip,
	      int udprcvsize, void **sa_ptr, socklen_t *sa_len);
int mcast_snd(char *host_name, char *service_port, char *ifname, char *ifip,
	      void **sa_ptr, socklen_t *sa_len);
const char *sock_ntop(struct sockaddr *sa);

#endif

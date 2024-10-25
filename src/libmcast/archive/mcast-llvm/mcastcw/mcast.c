/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id: mcast.c,v 1.1.1.1 2005/03/29 16:37:12 nieves Exp $
 */
/*
 * CYGWIN: Replaced file
 */
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>	/* IFNAMSIZ */
#include <sys/ioctl.h>	/* ioctl */
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "mcast.h"

static int udp_recv(char *service, int udprcvsize,
		    void **sa_ptr, socklen_t *sa_len);

int mcast_rcv(char *host_name, char *service_port, char *ifname, char *ifip,
	      int udprcvsize, void **sa_ptr, socklen_t *sa_len){

  int status = 0;
  int sfd = -1;
  struct ip_mreq mreq;
  struct ifreq ifreq;		/* for converting ifname */
  struct in_addr if_inaddr;	/* for converting ifip */

  mreq.imr_multiaddr.s_addr = inet_addr(host_name);

  if(ifip != NULL){
    if(inet_aton(ifip, &if_inaddr) == 0){
      errno = EINVAL;
      status = -1;
    }else
      memcpy(&mreq.imr_interface, &if_inaddr, sizeof(struct in_addr));
  }else if(ifname != NULL){
    strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
    ifreq.ifr_name[IFNAMSIZ - 1] = '\0';
    if(ioctl(sfd, SIOCGIFADDR, &ifreq) < 0)
      status = -1;
    else
      memcpy(&mreq.imr_interface,
	     &((struct sockaddr_in*)&ifreq.ifr_addr)->sin_addr,
	     sizeof(struct in_addr));
  }else
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

  sfd = udp_recv(service_port, udprcvsize, sa_ptr, sa_len);
  if(sfd < 0)
    return(-1);

  status = setsockopt(sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			&mreq, sizeof(struct ip_mreq));

  if(status != 0){
    close(sfd);
    sfd = -1;
  }

  return(sfd);
}

static int udp_recv(char *service, int udprcvsize,
		    void **sa_ptr, socklen_t *sa_len){

  struct sockaddr_in sa;
  uint16_t port;
  int on = 1;
  int sfd = -1;
  int status = 0;

  /*
   * FIX
   */
  port = atoi(service);

  /*
   * Fill in the sa structure to which we will bind.
   */ 
  memset(&sa, 0, sizeof(struct sockaddr_in));
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);

  if((sfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
     return(-1);

  status = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  if(status == 0){
    if(udprcvsize > 0)
      status = setsockopt(sfd, SOL_SOCKET, SO_RCVBUF,
			  &udprcvsize, sizeof(int));
  }

  if(status == 0)
     status = bind(sfd, (struct sockaddr*)&sa, sizeof(sa));

  if(status == 0){
    *sa_ptr = malloc(sizeof(struct sockaddr_in));
    if(*sa_ptr != NULL){
      memcpy(*sa_ptr, &sa, sizeof(struct sockaddr_in));
      *sa_len = sizeof(struct sockaddr_in);
    }else
      status = -1;
  }

  if(status != 0){
    if(sfd != -1){
      close(sfd);
      sfd = -1;
    }
  }

  return(sfd);
}

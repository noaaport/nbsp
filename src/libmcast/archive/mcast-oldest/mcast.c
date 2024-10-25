/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id: mcast.c,v 1.1.1.1 2005/03/29 16:37:12 nieves Exp $
 */
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include "mcast.h"

static int udp_client(char *host, char *service, 
		      void **sa_ptr, socklen_t *sa_len);

int mcast_recv(char *host_name, char *service_port, 
	       void **sa_ptr, socklen_t *sa_len){

  int status = 0;
  int sfd = -1;
  int on = 1;
  struct ip_mreq mreq;

  sfd = udp_client(host_name, service_port, sa_ptr, sa_len);
  if(sfd < 0)
    return(-1);

  status = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  if(status != -1)
    status = bind(sfd, *sa_ptr, *sa_len);

  if(status != -1){
    memcpy(&mreq.imr_multiaddr, &((struct sockaddr_in*)*sa_ptr)->sin_addr, 
	   sizeof(struct in_addr));

    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    status = setsockopt(sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			&mreq, sizeof(struct ip_mreq));
  }

  if(status == -1){
    /*
     * log_err("bind or sockopt error in mcast()");
     */
    close(sfd);
    sfd = -1;
  }

  return(sfd);
}

int mcast_send(char *host_name, char *service_port, 
	       void **sa_ptr, socklen_t *sa_len){

  int status = 0;
  int sfd = -1;
  u_char off = 0;

  sfd = udp_client(host_name, service_port, sa_ptr, sa_len);
  if(sfd < 0)
    return(-1);

  status = setsockopt(sfd, IPPROTO_IP, IP_MULTICAST_LOOP, &off, sizeof(off));

  if(status == -1){
    /*
     * log_err("bind or sockopt error in mcast()");
     */
    close(sfd);
    sfd = -1;
  }

  return(sfd);
}

static int udp_client(char *host, char *service, 
		      void **sa_ptr, socklen_t *sa_len){
  int status = 0;
  int sfd = -1;
  struct addrinfo hints, *res, *res0;
  
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  /*
   * This code is taken from the freebsd man page for gataddrinfo().
   */
  status = getaddrinfo(host, service, &hints, &res0);
  if(status != 0){
    /*
     * log_errx("udp_client error for %s, %s: %s", 
     *     host, service, gai_strerror(status));
     */

    goto end;
  }

  res = res0;
  while(res != NULL){
    sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sfd >= 0)
      break;          /* success */

    res = res->ai_next;
  }
  
  if (res == NULL){        /* errno set from final socket() */
    /*
     * log_err("udp_client error.");
     * log_err2(host, service);
     */

    goto end;
  }

  *sa_ptr = malloc(res->ai_addrlen);
  if(*sa_ptr != NULL){
    memcpy(*sa_ptr, res->ai_addr, res->ai_addrlen);
    *sa_len = res->ai_addrlen;
  }else{
    status = -1;
    /*
     * log_err("malloc error in udp_client()");
     */
  }

 end:

  freeaddrinfo(res0);
  
  if(status != 0){
    if(sfd != -1){
      close(sfd);
      sfd = -1;
    }
  }
    
  return(sfd);
}

const char *sock_ntop(struct sockaddr *sa){

  static char str[INET_ADDRSTRLEN];
  struct sockaddr_in *sin = (struct sockaddr_in *)sa;

  return(inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)));
}


/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id: mcast.c,v 1.20 2008/02/16 16:04:50 nieves Exp $
 */
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>	/* IFNAMSIZ */
#include <sys/ioctl.h>	/* ioctl */
#include <limits.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "mcast.h"

static int mcast_snd_setif(int fd, char *ifname, char *ifip);
static int mcast_snd_setttl(int fd, int ttl);
static int mcast_snd_setloop(int fd, int onoff);

int mcast_rcv_default(char *host_name, char *service_port,
                      void **sa_ptr, socklen_t *sa_len, int *gai_code){

  return(mcast_rcv(host_name, service_port, NULL, NULL, -1,
		   sa_ptr, sa_len, gai_code));
}

int mcast_snd_default(char *host_name, char *service_port, int ttl,
                      void **sa_ptr, socklen_t *sa_len, int *gai_code){

  return(mcast_snd(host_name, service_port, NULL, NULL,
		   ttl, sa_ptr, sa_len, gai_code));
}

int mcast_rcv(char *host_name, char *service_port, char *ifname, char *ifip,
	      int udprcvsize, void **sa_ptr, socklen_t *sa_len, int *gai_code){
  /*
   * The interface name (e.g., em0, fxp0, etc) can be given in ifname,
   * or the ip address in ifip. The ifip is used when given, else the ifname,
   * or the default (chosen by the kernel) if the interface is not specified.
   */
  int status = 0;
  int sfd = -1;
  int on = 1;
  struct ip_mreq mreq;
  struct ifreq ifreq;		/* for converting ifname */
  struct in_addr if_inaddr;	/* for converting ifip */

  sfd = udp_client(host_name, service_port, sa_ptr, sa_len, gai_code);
  if(sfd < 0)
    return(-1);

  if(udprcvsize > 0)
    status = setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, &udprcvsize, sizeof(int));

  /*
   * In Linux (and others), there is no SO_REUSEPORT but SO_REUSEADDR is
   * promoted to do the same thing for multicast adresses. Using SO_REUSEPORT
   * will permite to use the same port for all the addresses. It is not
   * needed by nbsp to receive the noaaport stream, but it will be needed
   * for the mcast redistribution using the IANNA assgined global ip's
   * with _one_ port.
   */
  if(status == 0){
#if defined(SO_REUSEPORT)
    status = setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
#else
    status = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
#endif
  }

  if(status == 0)
    status = bind(sfd, *sa_ptr, *sa_len);

  if(status == 0){
    memcpy(&mreq.imr_multiaddr, &((struct sockaddr_in*)*sa_ptr)->sin_addr, 
	   sizeof(struct in_addr));
    
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
  }

  if(status == 0)
    status = setsockopt(sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			&mreq, sizeof(struct ip_mreq));

  if(status != 0){
    close(sfd);
    sfd = -1;
  }

  return(sfd);
}

int udp_client(char *host, char *service, 
	       void **sa_ptr, socklen_t *sa_len, int *gai_code){
  int status = 0;
  int sfd = -1;
  struct addrinfo hints, *res, *res0;
  
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  /*
   * This code is taken from the freebsd man page for getaddrinfo().
   * (See also Stevens unp1-294.)
   */
  status = getaddrinfo(host, service, &hints, &res0);
  if(status != 0){
    /*
     * log_errx("udp_client error for %s, %s: %s", 
     *     host, service, gai_strerror(status));
     */
    *gai_code = status;

    return(-1);
  }
  *gai_code = 0;

  res = res0;
  while(res != NULL){
    sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sfd >= 0)
      break;          /* success */

    res = res->ai_next;
  }
  
  if (res == NULL){	/* errno set from final socket() */
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

int udp_server(char *host, char *service, socklen_t *sa_len, int *gai_code){
  /*
   * This function is not used by nbspd. It is used by the nbspudprecv
   * tool of the panfilter. It is similar to udp_client(), and is based
   * on the function given in Stevens unp1-296.
   */
  int sfd = -1;
  struct addrinfo hints, *res, *res0;
  int status = 0;

  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  
  status = getaddrinfo(host, service, &hints, &res0);
  if(status != 0){
    /*
     * log_errx("udp_client error for %s, %s: %s", 
     *     host, service, gai_strerror(status));
     */
    *gai_code = status;

    return(-1);
  }
  *gai_code = 0;

  res = res0;
  while(res != NULL){
    sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(sfd > 0){
      if(bind(sfd, res->ai_addr, res->ai_addrlen) == 0)
	break;			/* success */

      close(sfd);		/* bind error, close and try next one */
    }

    res = res->ai_next;
  }
  
  if (res == NULL){	/* errno from final socket() or bind() */
    /*
     * log_err("udp_server error.");
     * log_err2(host, service);
     */

    goto end;
  }

  /* return size of protocol address */
  if (sa_len != NULL)
    *sa_len = res->ai_addrlen; 

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

int mcast_snd(char *host_name, char *service_port, char *ifname, char *ifip,
	      int ttl, void **sa_ptr, socklen_t *sa_len, int *gai_code){

  int status = 0;
  int sfd = -1;
  u_char off = 0;

  sfd = udp_client(host_name, service_port, sa_ptr, sa_len, gai_code);
  if(sfd < 0)
    return(-1);

  status = setsockopt(sfd, IPPROTO_IP, IP_MULTICAST_LOOP, &off, sizeof(off));
  /*
   * To specify an interface other than the default, we have to call setsockopt
   * again with IP_MULTICAST_IF (See, Stevens, Vol. 1, p. 497).
   * The function mcast_snd_setif() does it.
   */
  if(status == 0)
    status = mcast_snd_setif(sfd, ifname, ifip);

  /*
   * If the TTL hop limit is not specified it defaults to 1, which restricts
   * the outgoing datagrams to the local subnet (p. 498).
   */
  if(status == 0)
    status = mcast_snd_setttl(sfd, ttl);

  if(status == 0)
    status = mcast_snd_setloop(sfd, 0);

  if(status == -1){
    close(sfd);
    sfd = -1;
  }

  return(sfd);
}

static int mcast_snd_setif(int fd, char *ifname, char *ifip){
  /*
   * Set the interface for outgoing packets.
   * This function is borrowed from Stevens unpv1 source code.
   * (and it is similar to what we do in mcast_rcv()).
   *
   * The interface name (e.g., em0, fxp0, etc) can be given in ifname,
   * or the ip address in ifip. The ifip is used when given, else the ifname,
   * or the default (chosen by the kernel) if the interface is not specified.
   */
  int status = 0;
  struct ifreq   ifreq;		/* for converting ifname */
  struct in_addr if_inaddr;     /* for converting ifip */

  if(ifip != NULL){
    if(inet_aton(ifip, &if_inaddr) == 0){
      errno = EINVAL;
      return(-1);
    }
  }else if(ifname != NULL) {
    strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
    ifreq.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(fd, SIOCGIFADDR, &ifreq) < 0)
      return(-1);

    memcpy(&if_inaddr, &((struct sockaddr_in*)&ifreq.ifr_addr)->sin_addr,
	   sizeof(struct in_addr));
  }else
    if_inaddr.s_addr = htonl(INADDR_ANY); /* remove previously set default */

  if(status == 0)
    status = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF,
			&if_inaddr, sizeof(struct in_addr));

  return(status);
}

static int mcast_snd_setttl(int sfd, int ttl){
  /*
   * From unp, page 498-499.
   */
  u_char uttl;

  if((ttl < 1) || ((unsigned int)ttl > UCHAR_MAX)){
    errno = EINVAL;
    return(-1);
  }

  uttl = (u_char)ttl;

  return(setsockopt(sfd, IPPROTO_IP, IP_MULTICAST_TTL, &uttl, sizeof(uttl)));
}

static int mcast_snd_setloop(int fd, int onoff){

  unsigned char flag;
  int status;

  flag = (unsigned char)onoff;
  status = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &flag, sizeof(flag));

  return(status);
}

const char *sock_ntop(struct sockaddr *sa){

  static char str[INET_ADDRSTRLEN];
  struct sockaddr_in *sin = (struct sockaddr_in *)sa;

  return(inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)));
}

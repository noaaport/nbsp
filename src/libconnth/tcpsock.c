/*
 * Copyright (c) 2002-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/param.h>			/* MAXHOSTNAMELEN */
#include "connect_timed.h"
#include "tcpsock.h"

static int tcp_server_accept_conn2(int s, int nonblock, int cloexec,
				   struct sockaddr *client,
				   socklen_t *clientlenp);

int tcp_server_open_conn(const char *host, const char *serv,
			 int backlog, int so_sndbuf, int so_rcvbuf,
			 socklen_t *addrlenp, int *gai_code){
  /*
   * Based on the FreeBSD manual page of getaddrinfo and also on unp1.
   * If host == NULL, this function uses the name returned by gethostname()
   * instead of passing NULL to getaddrinfo().  If host "*", then it
   * passes NULL to getaddrinfo(), but it sets ai_family to AF_INET
   * so only the ipv4address  0.0.0.0 are returned. If host == "**",
   * it also passes NULL to getaddrinfo but leaves ai_family to AF_UNSPEC
   * so that the first address returned is the ipv6 0::0 and that is used.
   */
  int status = 0;
  int s = -1;		/* file descriptor */
  const int on = 1;	/* for socket option */
  int bsize;		/* for so_sndbuf/so_rcvbuf */
  int s_flags;          /* for fcntl flags */
  char name[MAXHOSTNAMELEN + 1];
  int name_len = MAXHOSTNAMELEN + 1;
  struct addrinfo hints, *res, *res0;

  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  *gai_code = 0;

  if(host == NULL){
    if(gethostname(name, name_len) == -1)
      return(-1);
    else
      status = getaddrinfo(name, serv, &hints, &res0);
  }else if((strlen(host) == 1) && (host[0] == '*')){
    hints.ai_family = AF_INET;
    status = getaddrinfo(NULL, serv, &hints, &res0);
  }else if((strlen(host) == 2) && (host[0] == '*') && (host[1] == '*'))
    status = getaddrinfo(NULL, serv, &hints, &res0);
  else
    status = getaddrinfo(host, serv, &hints, &res0);

  if(status != 0){
    /*
     * log_errx("Error for %s, %s: %s", host, service, gai_strerror(status));
     */
    *gai_code = status;
    return(-1);
  }

  res = res0;
  while(res != NULL){
    if((s = socket(res->ai_family, res->ai_socktype, res->ai_protocol))
       == -1){
      status = -1;
    }
    if(status == 0)
      status = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if(status == 0){
      bsize = so_sndbuf;
      if(bsize > 0){
	status = setsockopt(s, SOL_SOCKET, SO_SNDBUF, &bsize, sizeof(bsize));
      }
    }

    if(status == 0){
      bsize = so_rcvbuf;
      if(bsize > 0){
	status = setsockopt(s, SOL_SOCKET, SO_RCVBUF, &bsize, sizeof(bsize));
      }
    }

    if(status == 0){
      /*
       * Set the socket not to block
       */
      if((s_flags = fcntl(s, F_GETFL, 0)) == -1)
	status = -1;
      else {
	s_flags |= O_NONBLOCK;
	if(fcntl(s, F_SETFL, s_flags) == -1)
	  status = -1;
      }
    }

    if(status == 0)
      status = bind(s, res->ai_addr, res->ai_addrlen);

    if(status == 0)
      status = listen(s, backlog);

    if(status == 0)
      break;

    if(s != -1){
      close(s);
      s = -1;
    }

    res = res->ai_next;
  }
  
  if(res != NULL){
    if(addrlenp != NULL)
      *addrlenp = res->ai_addrlen;   /* return size of protocol address */
  }  
  
  freeaddrinfo(res0);
  
  return(s);
}

int tcp_server_accept_conn(int s, struct client_options_st *clientopts,
			   struct sockaddr *addr, socklen_t *addrlenp){

  return(tcp_server_accept_conn2(s, clientopts->nonblock, clientopts->cloexec,
				 addr, addrlenp));
}

static int tcp_server_accept_conn2(int s, int nonblock, int cloexec,
				   struct sockaddr *client,
				   socklen_t *clientlenp){
  /*
   * Returns:
   *  (i) the file descriptor for communicating with the client, or
   *  (ii) -1 in case of error, or 
   *  (iii) -2 if accept() returns because it would block
   *        (the listening socket is non-blocking).
   *
   * If the parameter nonblock is 1, then the _client_ socket is
   * set nonblocking for read/write via fcntl.
   *
   * If cloexec is 1, the close on exec flag is set. 
   */
  int cfd = -1;
  int s_flags;
  int status = 0;

  cfd = accept(s, client, clientlenp);
  if(cfd == -1){
    /*
     * We use a non-blocking listening socket, so this function _can_ return
     * even if there is nothing at the other end (e.g. client has disconnected
     * as described in 15.6 of Steven's). We set this function to return (-2)
     * in this case, to distinguish from a real (-1) error return.
     */
    if((errno == EWOULDBLOCK) || (errno == ECONNABORTED) || (errno == EINTR))
      cfd = -2;
  }

  if(cfd < 0)
    return(cfd);

  if(nonblock == 1){
    /*
     * set the socket not to block
     */
    if((s_flags = fcntl(cfd, F_GETFL, 0)) == -1)
      status = -1;
    else{
      s_flags |= O_NONBLOCK;
      if(fcntl(cfd, F_SETFL, s_flags) == -1)
	status = -1;
    }
  }

  if((status == 0) && (cloexec == 1)){
    if(fcntl(cfd, F_SETFD, FD_CLOEXEC) == -1)
      status = -1;
  }

  if(status != 0){
    close(cfd);
    cfd = -1;
  }

  return(cfd);
}

int tcp_client_open_conn(const char *host, const char *serv,
			 int so_sndbuf, int so_rcvbuf, int *gai_code){

  return(tcp_client_open_conn_timed(host, serv, -1,
				    so_sndbuf, so_rcvbuf, gai_code));
}

int tcp_client_open_conn_timed(const char *host, const char *serv, int nsecs,
			       int so_sndbuf, int so_rcvbuf, int *gai_code){

  int cfd = -1;
  int status = 0;
  int bsize;
  struct addrinfo hints, *res, *res0;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if((status = getaddrinfo(host, serv, &hints, &res0)) != 0){
    /*
     * log_errx("Error for %s, %s: %s", host, service, gai_strerror(status));
     */
    *gai_code = status;
    return(-1);
  }
  *gai_code = 0;

  res = res0;
  while(res != NULL){
    if((cfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol))
       == -1)
      status = -1;

    if(status == 0){
      bsize = so_sndbuf;
      if(bsize > 0){
	status = setsockopt(cfd, SOL_SOCKET, SO_SNDBUF, &bsize, sizeof(bsize));
      }
    }

    if(status == 0){
      bsize = so_rcvbuf;
      if(bsize > 0){
	status = setsockopt(cfd, SOL_SOCKET, SO_RCVBUF, &bsize, sizeof(bsize));
      }
    }

    if(status == 0){
      if(nsecs < 0)
	status = connect(cfd, res->ai_addr, res->ai_addrlen);
      else 
	status = connect_timed(cfd, res->ai_addr, res->ai_addrlen, nsecs);
    }

    if(status == 0)
      break;

    if(cfd != -1){
      close(cfd);
      cfd = -1;
    }

    res = res->ai_next;
  }

  freeaddrinfo(res0);

  return(cfd);
}

char *get_peer_ip(int cfd){

  char *addr = NULL;
  struct sockaddr_in client;
  socklen_t len = sizeof(client);
  char *ip;

  if(getpeername(cfd, (struct sockaddr*)&client, &len) == 0)
    addr = inet_ntoa(client.sin_addr);

  if(addr == NULL)
    return(NULL);

  ip = malloc(strlen(addr) + 1);
  strncpy(ip, addr, strlen(addr) + 1);

  return(ip);
}

char *get_peer_name(int cfd){

  struct sockaddr_in client;
  struct hostent *ht;
  char *h_name;
  socklen_t len = sizeof(client);

  if(getpeername(cfd, (struct sockaddr*)&client, &len) != 0)
    return(NULL);

  ht = gethostbyaddr(&client.sin_addr, sizeof(struct in_addr),  AF_INET);
  if(ht == NULL)
    return(NULL);

  h_name = malloc(strlen(ht->h_name) + 1);
  if(h_name == NULL)
    return(NULL);

  strncpy(h_name, ht->h_name, strlen(ht->h_name) + 1);

  return(h_name);
}

/*
 * Copyright (c) 2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>	/* gai_strerror() */
#include "mcast.h"	/* nbsp/src */

/*
 * We set the default host to 0.0.0.0 instead of NULL, so that if the
 * the user does not specify explcitly a host name or ip, the default
 * protocol returned by udp_server (via getaddrinfo) is ipv4.
 * (See unp1-291.)
 */
struct {
  char *host;		/* name or ip */
  char *service;	/* port */
  int size;		/* size of expected datagrams */
} g = {"0.0.0.0", NULL, 512};

static int loop(int s, socklen_t sa_len);
static int udprecv(void);

int main(int argc, char **argv){

  char *optstr = "s:";
  char *usage = "nbspudprecv [-s size] [host] <port>";
  int status = 0;
  int c;

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 's':
      g.size = atoi(optarg);
      break;
    default:
      status = 1;
      errx(1, "%s", usage);
      break;
    }
  }

  if(optind == argc - 1)
    g.service = argv[optind++];
  else if(optind == argc - 2){
    g.host = argv[optind++];
    g.service = argv[optind++];
  }else
    errx(1, "%s", usage);

  status = udprecv();

  return(status != 0 ? 1 : 0);
}

static int udprecv(void){

  socklen_t sa_len;
  int sfd = -1;
  int status = 0;
  int gai_code;

  sfd = udp_server(g.host, g.service, &sa_len, &gai_code);
  if(sfd < 0){
    if(gai_code == 0)
      err(1, "Error in udp_server()");
    else
      errx(1, "Error in gataddrinfo() in udp_server(). %s",
	   gai_strerror(gai_code));

    return(-1);
  }

  status = loop(sfd, sa_len);    /* receive and print */

  return(status);
}

static int loop(int s, socklen_t sa_len){

  static struct sockaddr *sa = NULL;
  char *buf = NULL;
  socklen_t len;
  ssize_t n;
  int status = 0;

  sa = malloc(sa_len);
  if(sa == NULL)
    err(1, "Error from malloc()");

  buf = malloc(g.size);
  if(buf == NULL)
    err(1, "Error from malloc()");

  while(status == 0){
    len = sa_len;
    memset(buf, '\0', g.size - 1);
    n = recvfrom(s, buf, g.size, 0, sa, &len);
    if(n == -1)
      status = -1;
    else
      fprintf(stdout, "%s %s\n", sock_ntop(sa), buf);
  }

  if(status == -1)
    err(1, "Error from recvfrom()");

  return(status);
}

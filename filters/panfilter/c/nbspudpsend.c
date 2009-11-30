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

#define STRSIZE		512

struct {
  char *opt_host;
  char *opt_service;
  char *opt_str;
} g = {NULL, NULL, NULL};

static int udpsend(void);

int main(int argc, char **argv){

  char *optstr = "e:";
  char *usage = "nbspudpsend [-e str] <host> <port>";
  int c;
  int status = 0;

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'e':
      g.opt_str = optarg;
      break;
    case 'h':
    default:
      status = 1;
      errx(1, usage);
      break;
    }
  }

  if(optind != argc - 2)
    errx(1, usage);

  g.opt_host = argv[optind++];
  g.opt_service = argv[optind++];

  /* atexit(cleanup); */

  status = udpsend();

  return(status != 0 ? 1 : 0);
}

static int udpsend(void){

  int fd;
  void *addr;
  struct sockaddr *readeraddr;
  socklen_t readeraddr_len;
  char str[STRSIZE];
  int status = 0;
  int gai_code;

  fd = udp_client(g.opt_host, g.opt_service,
		  &addr, &readeraddr_len, &gai_code);
  if(fd < 0){
    if(gai_code == 0)
      err(1, "Error in udp_client()");
    else
      errx(1, "Error in gataddrinfo() in udp_client(). %s",
	   gai_strerror(gai_code));

    return(-1);
  }

  readeraddr = addr;

  if(g.opt_str != NULL){
    if(sendto(fd, g.opt_str, strlen(g.opt_str), 0,
	      readeraddr, readeraddr_len) == -1)
      status = -1;
  }else{
    while((fgets(str, STRSIZE, stdin) != NULL) && (status == 0)){
      if(sendto(fd, str, strlen(str), 0, readeraddr, readeraddr_len) == -1)
	status = -1;
    }
  }
  if(status == -1)
    err(1, "Error in sendto() %s",  sock_ntop(readeraddr));
  
  free(readeraddr);

  return(0);
}

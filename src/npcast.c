/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>		/* the two below need this */
#include <netinet/in.h>		/* INET_ADDRSTRLEN */
#include <net/if.h>		/* IFNAMSIZ */
#include <netdb.h>		/* gai_strerror codes */
#include "util.h"
#include "strsplit.h"
#include "err.h"
#include "mcast.h"
#include "npcast.h"

#define NP_DELIM_STR		": \t"

static struct npcast_st gnpcast = {NULL, 0};
static fd_set grdset;
static int gmaxfd;
static  struct strsplit_st *gip = NULL;
static  struct strsplit_st *gport = NULL;
static  char *gifname = NULL;
static  char *gifip = NULL;

static int np_init(char *np_ip, char *np_port, char *ifname, char *ifip,
		   int udprcvsize);
static int np_open_channels(void);
static void np_close_channels(void);
static int open_channel(int channel_id);
static void close_channel(int channel_id);

static int np_init(char *np_ip, char *np_port, char *ifname, char *ifip,
	    int udprcvsize){
  /*
   * Here np_ip should be a string of the form
   *
   * np_ip = "224.0.1.1 224.0.1.2 ..."
   *
   * and similarly for np_port. The strings can be set in the runtime
   * configuration file. 
   *
   * Returns:
   *  0 => ok
   *  1 => ips and/or ports not specified, or not the same number, or too many.
   * -1 => memory error.
   */
  int status = 0;
  int i;

  if((valid_str(np_ip) == 0) || (valid_str(np_port) == 0))
    return(1);

  if(valid_str(ifname)){
    gifname = malloc(IFNAMSIZ);
    if(gifname == NULL){
      status = -1;
      goto end;
    }else{
      strncpy(gifname, ifname, IFNAMSIZ);
      gifname[IFNAMSIZ - 1] = '\0';
    }
  }

  if(valid_str(ifip)){
    gifip = malloc(INET_ADDRSTRLEN);
    if(gifip == NULL){
      status = -1;
      goto end;
    }else{
      strncpy(gifip, ifip, INET_ADDRSTRLEN);
      gifip[INET_ADDRSTRLEN - 1] = '\0';
    }
  }

  if(np_ip != NULL){
    gip = strsplit_create(np_ip, NP_DELIM_STR, STRSPLIT_FLAG_IGNEMPTY);
    if(gip == NULL){
      status = -1;
      goto end;
    }
  }

  if(np_port != NULL){
    gport = strsplit_create(np_port, NP_DELIM_STR, STRSPLIT_FLAG_IGNEMPTY);
    if(gport == NULL){
      status = -1;
      goto end;
    }
  }

  if((gip->argc == 0) || (gport->argc == 0) || (gip->argc != gport->argc)){
    status = 1;
    goto end;
  }

  if(gip->argc > NPCAST_NUM_CHANNELS){
    status = 1;
    goto end;
  }
  gnpcast.numchannels = gip->argc;	/* used */

  gnpcast.channel = calloc(NPCAST_NUM_CHANNELS,
			   sizeof(struct npcast_channel_st));
  if(gnpcast.channel == NULL){
    status = -1;
    goto end;
  }

  /*
   * Disable all, and enable only those specified.
   */
  for(i = 0; i <= NPCAST_NUM_CHANNELS - 1; ++i){
    gnpcast.channel[i].f_enable = 0;
    gnpcast.channel[i].sfd = -1;
    gnpcast.channel[i].sa = NULL;
    gnpcast.channel[i].sender_sa = NULL;
  }

  for(i = 0; i <= gnpcast.numchannels - 1; ++i){
    gnpcast.channel[i].f_enable = 1;
    gnpcast.channel[i].ip = gip->argv[i];
    gnpcast.channel[i].port = gport->argv[i];
    gnpcast.channel[i].udprcvsize = udprcvsize;
  }

 end:

  if(status != 0){
    if(gifname != NULL){
      free(gifname);
      gifname = NULL;
    }

    if(gifip != NULL){
      free(gifip);
      gifip = NULL;
    }

    if(gip != NULL){
      strsplit_delete(gip);
      gip = NULL;
    }

    if(gport != NULL){
      strsplit_delete(gport);
      gport = NULL;
    }

    if(gnpcast.channel != NULL){
      free(gnpcast.channel);
      gnpcast.channel = NULL;
    }
  }

  if(status == 0){
    for(i = 0; i <= NPCAST_NUM_CHANNELS - 1; ++i){
      if(gnpcast.channel[i].f_enable)
	log_info("Enabling %s:%s", 
		 gnpcast.channel[i].ip, gnpcast.channel[i].port);
    }
  }

  return(status);
}

void np_close(void){

  if(gifname != NULL){
    free(gifname);
    gifname = NULL;
  }

  if(gifip != NULL){
    free(gifip);
    gifip = NULL;
  }

  if(gip != NULL){
    strsplit_delete(gip);
    gip = NULL;
  }

  if(gport != NULL){
    strsplit_delete(gport);
    gport = NULL;
  }

  if(gnpcast.channel != NULL){
    np_close_channels();
    free(gnpcast.channel);
    gnpcast.channel = NULL;
  }
}

int np_open(char *np_ip, char *np_port, char *ifname, char *ifip,
	    int udprcvsize){

  int status;

  status = np_init(np_ip, np_port, ifname, ifip, udprcvsize);
  if(status == 0)
    status = np_open_channels();

  return(status);
}

static int np_open_channels(void){
  /*
   * Returns:
   *  0 => no errors
   * -1 => system error from open_channel()
   *  1 => all channels were disabled
   */
  int i;
  int status = 0;

  FD_ZERO(&grdset);
  gmaxfd = -1;

  for(i = 0; i <= NPCAST_NUM_CHANNELS - 1; ++i){
    if(get_npcast_channel_enable(i) == 0)
      continue;

    status = open_channel(i);

    if(status != 0)
      break;
      
    if(gnpcast.channel[i].sfd > gmaxfd)
      gmaxfd = gnpcast.channel[i].sfd;
  }

  if((status == 0) && (gmaxfd == -1))
    status = 1;

  return(status);
}

static void np_close_channels(void){

  int i;

  for(i = 0; i <= NPCAST_NUM_CHANNELS - 1; ++i)
    close_channel(i);
}

static int open_channel(int id){

  char *ip;
  char *port;
  int udprcvsize;
  int sfd = -1;
  /* struct sockaddr *sa = NULL; */
  void *sa = NULL;
  socklen_t sa_len;
  struct sockaddr *sender_sa = NULL;
  int gai_code;
  int status = 0;

  assert(id <= NPCAST_NUM_CHANNELS - 1);

  ip = gnpcast.channel[id].ip;
  port = gnpcast.channel[id].port;
  udprcvsize = gnpcast.channel[id].udprcvsize;

  sfd = mcast_rcv(ip, port, gifname, gifip, udprcvsize,
		  &sa, &sa_len, &gai_code);

  if(sfd < 0){
    if((gai_code == 0) || (gai_code == EAI_SYSTEM))
      return(-1);
    else{
      log_errx("getaddrinfo gai_code %d: %s",
	       gai_code, gai_strerror(gai_code));
      return(1);
    }
  }

  sender_sa = malloc(sa_len);
  if(sender_sa == NULL)
    status = -1;

  if(status == 0){
    gnpcast.channel[id].sfd = sfd;
    gnpcast.channel[id].sa = (struct sockaddr*)sa;
    gnpcast.channel[id].sa_len = sa_len;
    gnpcast.channel[id].sender_sa = sender_sa;
    gnpcast.channel[id].sender_sa_len = sa_len;
  }else{
    if(sfd != -1)
      close(sfd);

    if(sa != NULL)
      free(sa);

    if(sender_sa != NULL)
      free(sender_sa);
  }

  return(status);
}

static void close_channel(int id){

  assert(id <= NPCAST_NUM_CHANNELS - 1);

  if(gnpcast.channel[id].sfd != -1)
    close(gnpcast.channel[id].sfd);
  
  if(gnpcast.channel[id].sa != NULL)
    free(gnpcast.channel[id].sa);

  if(gnpcast.channel[id].sender_sa != NULL)
    free(gnpcast.channel[id].sender_sa);

  gnpcast.channel[id].sfd = -1;
  gnpcast.channel[id].sa_len = 0;
  gnpcast.channel[id].sa = NULL;
  gnpcast.channel[id].sender_sa = NULL;
  gnpcast.channel[id].sender_sa_len = 0;
}
	
ssize_t recvfrom_channel_nowait(int id, void *buf, size_t len){
  
  ssize_t n;
  int sfd;

  assert(id <= NPCAST_NUM_CHANNELS - 1);

  sfd = gnpcast.channel[id].sfd;

  n = recvfrom(sfd, buf, len, MSG_DONTWAIT, gnpcast.channel[id].sender_sa, 
	       &gnpcast.channel[id].sender_sa_len);

  return(n);
}

ssize_t recvfrom_channel_timed(int id, void *buf, size_t len,
			      unsigned int timeout_secs){
  int status;
  ssize_t n = 0;
  int fd;
  struct timeval timeout;
  struct timeval *tvp = NULL;
  fd_set fdset;

  if(timeout_secs > 0){
    timeout.tv_sec = timeout_secs;
    timeout.tv_usec = 0;
    tvp = &timeout;
  }

  fd = gnpcast.channel[id].sfd;
  FD_ZERO(&fdset);
  FD_SET(fd, &fdset);

  status = select(fd + 1, &fdset, NULL, NULL, tvp);
  if(status == -1)
    n = -1;
  else if(status > 0){
    if(FD_ISSET(fd, &fdset))
      n = recvfrom_channel_nowait(id, buf, len); 
  }

  return(n);
}

int np_select(unsigned int timeout_secs){

  int status;
  int i;
  struct timeval timeout;
  struct timeval *tvp = NULL;

  if(timeout_secs > 0){
    timeout.tv_sec = timeout_secs;
    timeout.tv_usec = 0;
    tvp = &timeout;
  }

  for(i = 0; i <= NPCAST_NUM_CHANNELS - 1; ++i){
    if(gnpcast.channel[i].sfd != -1)
      FD_SET(gnpcast.channel[i].sfd, &grdset);
  }

  status = select(gmaxfd + 1, &grdset, NULL, NULL, tvp);

  return(status);
}

ssize_t np_read_channel(int id, void *buf, size_t len){

  ssize_t n = 0;
  int sfd;

  sfd = gnpcast.channel[id].sfd;
  if((sfd != -1) && FD_ISSET(sfd, &grdset))
    n = recvfrom_channel_nowait(id, buf, len); 
  
  return(n);
}

int get_npcast_channel_enable(int i){

  return(gnpcast.channel[i].f_enable);
}

int get_npcast_numchannels(void){

  return(gnpcast.numchannels);
}

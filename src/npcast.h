/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef NPCAST_H
#define NPCAST_H

#include <sys/socket.h>
#include "defaults.h"

#if 0
#define NPCAST_MASK_CHANNEL0	1
#define NPCAST_MASK_CHANNEL1	2
#define NPCAST_MASK_CHANNEL2	4
#define NPCAST_MASK_CHANNEL3	8
#endif

struct npcast_channel_st {
  char *ip;
  char *port;
  int udprcvsize;
  int id;
  int f_enable;
  int sfd;
  struct sockaddr *sa;
  socklen_t sa_len;
  struct sockaddr *sender_sa;
  socklen_t sender_sa_len;
};

struct npcast_st {
  struct npcast_channel_st *channel;
  int numchannels;
};

int np_open(char *np_ip, char *np_port, char *ifname, char *ifip,
	    int udprcvsize);
void np_close(void);
ssize_t recvfrom_channel_nowait(int channel_id, void *buf, size_t len);
ssize_t recvfrom_channel_timed(int id, void *buf, size_t len,
			       unsigned int timeout_secs);
int get_npcast_channel_enable(int i);
int get_npcast_numchannels(void);    /* number of channels enabled */

/*
 * Not used when there is a reader for each channel.
 */
int np_select(unsigned int timeout_secs);
ssize_t np_read_channel(int id, void *buf, size_t len);

#endif

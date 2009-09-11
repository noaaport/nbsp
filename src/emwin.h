/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef EMWIN_H
#define EMWIN_H

#include <sys/types.h>
#include <sys/param.h>		/* MAXPATHLEN */

#define EMWIN_HEADER_DATESIZE	22
#define EMWIN_HEADER_FNAMESIZE	12
#define EMWIN_HEADER_SIZE	80
#define EMWIN_NULL_SIZE		6
#define EMWIN_DATA_SIZE		1024
#define EMWIN_PACKET_SIZE	1116

struct emwin_packet_st {
  char fname[EMWIN_HEADER_FNAMESIZE + 1];
  char header_date[EMWIN_HEADER_DATESIZE + 1];
  char packet[EMWIN_PACKET_SIZE];
  int packet_size;
  int  fd;		/* fd of the data file */
  int parts_total;
  int last_part_size;
  int part_number;
  int checksum;
};

/*
 * In the threaded version of libconn, in order to create the queues
 * we need to know the size of the fpathout files that are produced
 * by the emwin filter. However, that is impossible to determine because
 * the fpathout depends on the runtime rc file, which can be modified
 * by the user at any time. The best we can do is to use the maximum
 * allowed by the OS, which is likely to be a lot of space wasted in the
 * queues, but there is no other choice guaranteed to work.
 */
#define EMWIN_FPATHOUT_SIZE	MAXPATHLEN

/*
 * This is the data that is sent to the emwin client queues.
 */
struct emwin_queue_info_st {
  char emwinfname[EMWIN_HEADER_FNAMESIZE + 1];
  char fpathout[EMWIN_FPATHOUT_SIZE + 1];
};

int init_emwin_packet_st(struct emwin_packet_st *ep,
			 char *fpath, char *emwinfname);
int emwin_queue_info_get_size(void);
void free_emwin_packet_st(struct emwin_packet_st *ep);
int build_emwin_packet(struct emwin_packet_st *ep);
int send_emwin_packet(int fd, struct emwin_packet_st *ep, int timeout_ms,
		      int retry);

#endif

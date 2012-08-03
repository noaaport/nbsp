/*
 * Copyright (c) 2009 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SLAVET_H
#define SLAVET_H

#include <time.h>
#include <sys/stat.h>	/* mode_t */
#include <pthread.h>

/*
 * These are the functions to construct the table of master servers
 * of the network slave threads. The input is a "masterservers" string
 * of the form
 *
 * <protocol>,<server>,<port>,<options>|<protocol>,<server>,<port>,<options>|..
 *
 * There are five options (the int's in slave_options_st below). Network
 * slaves have eight fields, infifo's have only seven (no port).
 */
#define SLAVE_STRING_SEP1 "| \t\n"
#define SLAVE_STRING_SEP2 ",:"

#define SLAVE_NUM_OPTIONS 5
#define SLAVE_NET_STRING_FIELDS 8
#define SLAVE_IN_STRING_FIELDS 7

struct slave_options_st {
  mode_t infifo_mode;
  char *infifo_grp;
  char *slavestatsfile;
  char *slave_masterport;
  int slave_read_timeout_secs;     /* timeout when slave reads from master */
  int slave_read_timeout_retry;
  int slave_reopen_timeout_secs;   /* sleep secs before reopening connection */
  int slave_so_rcvbuf;
  int slave_stats_logperiod_secs;
};

struct slave_stats_st {
  unsigned int connect_errors;
  time_t ctime;                 /* last connection time */
  unsigned int errors_ctime;    /* since connection time to reset time */
  unsigned int packets_ctime;
  double bytes_ctime;
  time_t rtime;                 /* time of last reset */
  unsigned int errors_rtime;    /* errors since reset time */
  unsigned int packets_rtime;   /* packets received since reset time */
  double bytes_rtime;
};

/*
 * The last element of the slave_element_st is for each thread to
 * store anything. It is used as the struct packet_info_st *packetinfo
 * memory pool for nbs2 slaves, which is used to be a static variable
 * in slavefp/reader.c when there was only on thread.
 */
struct slave_element_st {
#define SLAVETYPE_NBS1 1		/* send file content */
#define SLAVETYPE_NBS2 2		/* send file fpath */
#define SLAVETYPE_INFIFO 3
  int slavetype;		/* nbs1, nbs2 or infifo */
  char *mastername;             /* master host for slave net mode */
  char *masterport;
  char *infifo;                 /* input fifo for slave input mode */
  struct slave_options_st options;
  /* variables */
  int f_slave_thread_created;
  pthread_t slave_thread_id;
  int slave_fd;			/* managed by each slave thread */
  int slavenbs_reader_index;    /* see comment below */
  void *info;                   /* packetinfo memory pool for nbs2 slaves */
  struct slave_stats_st stats;
};

/*
 * The nbs slaves use the pctl and that requires one channel for each of them.
 * Thus we must keep track of their number, and also keep the index of their
 * channel in the pctl table (slavenbs_reader_index).
 */
struct slave_table_st {
  struct slave_element_st *slave;
  int numslaves;
  int num_slavenbs_readers;
};

int slave_table_create(struct slave_table_st **slavet,
		       char *s,
		       struct slave_options_st *defaults);
void slave_table_destroy(struct slave_table_st *slavet);

/*
 * stats functions -
 */
void slave_stats_init(struct slave_element_st *slave);
void slave_stats_connect(struct slave_element_st *slave);
void slave_stats_reset(struct slave_element_st *slave);
void slave_stats_update_errors(struct slave_element_st *slave);
void slave_stats_update_connect_errors(struct slave_element_st *slave);
void slave_stats_update_packets(struct slave_element_st *slave,
				size_t packet_size);
void slave_stats_report(struct slave_element_st *slave);

#endif

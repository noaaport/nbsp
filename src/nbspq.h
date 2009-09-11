/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef NBSPQ_H
#define NBSPQ_H

#include <sys/types.h>

int init_nbspq(void);
void kill_nbspq(void);
int e_nbspq_snd_filter(void *packet, size_t packet_size);
int e_nbspq_snd_server(void *packet, size_t packet_size);
int e_nbspq_snd1(int type, void *packet, size_t packet_size);
int e_nbspq_rcv_filter(void **packet, size_t *packet_size);
int e_nbspq_rcv_server(void **packet, size_t *packet_size);
int e_nbspq_rcv1(int type, void **packet, size_t *packet_size);
void e_nbspq_report_quota(void);
void e_nbspq_dbstats(void);

#endif

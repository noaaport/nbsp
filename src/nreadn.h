/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef NREADN_H
#define NREADN_H

#include <sys/types.h>

/* typedef of pointer to a function like get_quit_flag() */
typedef int (*retry_interrupt_t)(void);

/* public function to set pointer to the retry-interrupt function */
void init_readn(retry_interrupt_t function);

ssize_t readn(int fd, void *buf, size_t size, unsigned int secs, int retry);
ssize_t writen(int fd, void *buf, size_t size, unsigned int secs, int retry);

ssize_t readm(int fd, void *buf, size_t size, unsigned int msecs, int retry);
ssize_t writem(int fd, void *buf, size_t size, unsigned int msecs, int retry);
ssize_t read1(int fd, void *buf, size_t size, unsigned int msecs, int retry);
ssize_t write1(int fd, void *buf, size_t size, unsigned int msecs, int retry);

ssize_t sreadm(int fd, void *buff, size_t size,
	       unsigned int msecs, int retry, int *eof);
ssize_t sreadn(int fd, void *buff, size_t size,
	       unsigned int secs, int retry, int *eof);

ssize_t readn_fifo(int fd, void *buf, size_t size, unsigned int secs);
ssize_t readm_fifo(int fd, void *buf, size_t size, unsigned int msecs);

ssize_t dpgets(int fd, char *buf, size_t size);

#endif

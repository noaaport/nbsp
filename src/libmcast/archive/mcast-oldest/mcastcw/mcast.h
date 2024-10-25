/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id: mcast.h,v 1.1.1.1 2005/03/29 16:37:13 nieves Exp $
 */
#ifndef MCAST_H
#define MCAST_H

#include <sys/socket.h>

int mcast(char *host, char *service, void **sa_ptr, socklen_t *sa_len);

#endif

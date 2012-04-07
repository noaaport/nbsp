/*
 * Copyright (c) 2002-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef CONNECT_TIMED_H

int connect_timed(int sockfd, const struct sockaddr *saptr,
		  socklen_t salen, int nsecs);

#endif

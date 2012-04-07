/*
 * Copyright (c) 2002-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef LIBCONNTH_SOCK_H
#define LIBCONNTH_SOCK_H

#include <sys/types.h>
#include "client_options.h"

int server_open_conn(char *name, char *group, int backlog);
int server_accept_conn(int fd, pid_t *pid,
		       struct client_options_st *clientopts);
int client_open_conn(char *server_name, char *client_basename);
int chgrpmode(char *name, char *group, mode_t mode);

#endif

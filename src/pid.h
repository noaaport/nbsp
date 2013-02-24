/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

#include <sys/types.h>

/* pid.c */
int create_pidfile(char *name, mode_t mode);
int remove_pidfile(char *name);

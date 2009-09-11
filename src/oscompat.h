/*
 * Copyright (c) 2005-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef OSCOMPAT_H
#define OSCOMPAT_H

#include <time.h>

int oscompat_clock_gettime(struct timespec *tp);

#endif

/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef READER_H
#define READER_H

#include <pthread.h>
#include "defaults.h"

int spawn_readers(void);
void kill_reader_threads(void);

#endif

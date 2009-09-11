/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef NBSPRE_H
#define NBSPRE_H

#include "ure.h"

int init_nbsp_regex(void);
void free_nbsp_regex(void);
int np_regex_match(char *name);
int filterq_regex_match(char *name);
int serverq_regex_match(char *name);
int nbs1_regex_match(char *name);
int nbs2_regex_match(char *name);
int emwin_regex_match(char *name);
int savez_regex_match(char *name);
int rtxdb_regex_match(char *name);

#endif


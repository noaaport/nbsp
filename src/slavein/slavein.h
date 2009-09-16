/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SLAVEIN_H
#define SLAVEIN_H

#include "../slavet.h"

int slavein_init(struct slave_element_st *slave);
void slavein_cleanup(struct slave_element_st *slave);
int slavein_loop(struct slave_element_st *slave);

#endif

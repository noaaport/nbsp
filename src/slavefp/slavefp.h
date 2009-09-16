/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SLAVEFP_H
#define SLAVEFP_H

#include "../slavet.h"

int slavenet_init_nbs2(struct slave_element_st *slave);
void slavenet_cleanup_nbs2(struct slave_element_st *slave);
int slavenet_loop_nbs2(struct slave_element_st *slave);

#endif

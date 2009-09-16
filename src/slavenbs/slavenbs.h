/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SLAVENBS_H
#define SLAVENBS_H

#include "../slavet.h"

int slavenet_init_nbs1(struct slave_element_st *slave);
void slavenet_cleanup_nbs1(struct slave_element_st *slave);
int slavenet_loop_nbs1(struct slave_element_st *slave);

#endif

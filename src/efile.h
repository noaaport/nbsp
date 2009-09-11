/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef EFILE_H
#define EFILE_H

#include <fcntl.h>
#include "pctl.h"

int e_open_product_file(struct pctl_element_st *pce);
int e_dir_exists(char *dirname);

#endif

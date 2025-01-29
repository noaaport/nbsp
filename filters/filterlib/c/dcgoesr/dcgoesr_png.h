/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCGOESR_PNG_H
#define DCGOESR_PNG_H

#include <stdio.h>

int write_data_png(FILE *fp, unsigned char *data,
		   int linesize, int numlines);
#endif

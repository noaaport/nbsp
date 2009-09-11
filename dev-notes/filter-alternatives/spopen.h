/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SPOPEN_H
#define SPOPEN_H

#include <stdio.h>
#include <sys/types.h>

int spopen(char *command, char *type, FILE *fp[2], pid_t *pid);
int spclose(FILE *fp[2], pid_t pid);

#endif

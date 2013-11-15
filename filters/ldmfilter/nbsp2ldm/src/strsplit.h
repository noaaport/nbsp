/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id: strsplit.h,v bc2b0f5cffa5 2013/11/12 22:22:28 nieves $
 */
#ifndef STRSPLIT_H
#define STRSPLIT_H

#define STRSPLIT_FLAG_INCEMPTY	0
#define STRSPLIT_FLAG_IGNEMPTY	1

struct strsplit_st {
  char *sp;
  int argc;
  char **argv;
};

struct strsplit_st *strsplit_create(char *s, char *delim, int flags);
void strsplit_delete(struct strsplit_st *strp);
struct strsplit_st *strsplit_recreate(char *s, char *delim, int flags,
				      struct strsplit_st *strp);
#endif

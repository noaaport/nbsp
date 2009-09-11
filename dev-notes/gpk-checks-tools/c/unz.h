/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef UNZ_H
#define UNZ_h

int unz(char *out, int *outlen, char *in, int inlen);
int zip(char **out, int *outlen, char *in, int inlen);

#endif

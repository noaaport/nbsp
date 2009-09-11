/*
 * Copyright (c) 2005-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef URE_H
#define URE_H

/*
 * If the uwildregex is NULL then everything is accepted; if it is empty
 * nothing is acepted. Otherwise each pattern is applied,
 * left to right. Whenever there is match, a match variable is set to 0, or
 * 1 if the pattern is negated. The candidate is accepted iff at the end
 * there was at least one match and the match variable is 0.
 *
 * The function uwildregex_match() returns:
 *
 *   0 => null or the right-most matching pattern is not negated  (accept)
 *   1 => empty, or no match found, or the right-most matching pattern
 *        is negated     (reject)
 *  -1 => error
 *
 *   If the uwildregex is invalid it is treated as NULL.
 */

#include <regex.h>

#define UWILDREGEX_ERRBUFFER_SIZE 128

#define UWILDREGEX_FLAG_VALID   0     /* non-null and non-empty */
#define UWILDREGEX_FLAG_NULL	1
#define UWILDREGEX_FLAG_EMPTY   2
#define UWILDREGEX_FLAG_INVALID 3 /* non-null, non-empty but error compiling */
#define UWILDREGEX_DELIM	","
#define UWILDREGEX_NEGCHAR	'!'

struct uwildregex_st {
  regex_t *re;			/* An array of regex_t */
  int *negate_flag;		/* Whether the pattern is negated */
  int nre;			/* Number of elements */
  int flag;			/* valid, null, empty, invalid */
  int errcode;
  char *errbuffer;
  size_t errbuffer_size;
};

struct uwildregex_st *uwildregex_create(void);
int uwildregex_init(struct uwildregex_st *r, char *uwildregex);
void uwildregex_free(struct uwildregex_st *r);
int uwildregex_match(struct uwildregex_st *r, char *str);

#endif

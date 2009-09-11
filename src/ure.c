/*
 * Copyright (c) 2005-2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include "strsplit.h"
#include "ure.h"

static void uwildregex_error(struct uwildregex_st *r, regex_t *reg);

struct uwildregex_st *uwildregex_create(void){

  struct uwildregex_st *r;

  r = malloc(sizeof(struct uwildregex_st));
  if(r == NULL)
    return(NULL);

  r->errbuffer = malloc(UWILDREGEX_ERRBUFFER_SIZE);
  if(r->errbuffer == NULL){
    free(r);
    return(NULL);
  }

  r->re = NULL;
  r->negate_flag = NULL;
  r->nre = 0;
  r->flag = UWILDREGEX_FLAG_NULL;
  r->errbuffer_size = UWILDREGEX_ERRBUFFER_SIZE;

  return(r);
}

void uwildregex_free(struct uwildregex_st *r){

  int i;

  if(r->re != NULL){
    for(i = 0; i < r->nre; ++i){
      regfree(&r->re[i]);
    }
    free(r->re);
  }

  if(r->negate_flag != NULL)
    free(r->negate_flag);

  if(r->errbuffer != NULL)
    free(r->errbuffer);

  free(r);
}

int uwildregex_init(struct uwildregex_st *r, char *uwildregex){
  /*
   * Returns:
   *   0 => no error
   *   1 => error compiling any of the patterns
   *  -1 => error outside of regex lib 
   */
  int status = 0;
  struct strsplit_st *uwildsplit;
  int i;
  char *p;

  if(uwildregex == NULL){
    r->flag = UWILDREGEX_FLAG_NULL;
    return(0);
  }else if(uwildregex[0] == '\0'){
    r->flag = UWILDREGEX_FLAG_EMPTY;
    return(0);
  }

  uwildsplit = strsplit_create(uwildregex, UWILDREGEX_DELIM,
			       STRSPLIT_FLAG_IGNEMPTY);
  if(uwildsplit == NULL)
    return(-1);

  /*
   * Create the regex_t array
   */
  r->re = malloc(sizeof(regex_t) * uwildsplit->argc);
  if(r->re == NULL){
    strsplit_delete(uwildsplit);
    return(-1);
  }

  r->negate_flag = malloc(sizeof(int) * uwildsplit->argc);
  if(r->negate_flag == NULL){
    free(r->re);
    strsplit_delete(uwildsplit);
    return(-1);
  }

  for(i = 0; i < uwildsplit->argc; ++i){
    p = uwildsplit->argv[i];
    r->negate_flag[i] = 0;
    if(*p == UWILDREGEX_NEGCHAR){
      r->negate_flag[i] = 1;
      /* Point after the ! */
      ++p;
    }
    r->errcode = regcomp(&r->re[i], p, REG_EXTENDED | REG_NOSUB);
    if(r->errcode != 0){
      r->flag = UWILDREGEX_FLAG_INVALID;
      status = 1;
      break;
    }
  }

  if(status != 0){
    uwildregex_error(r, &r->re[i]);
    free(r->re);
    r->re = NULL;
    return(status);
  }

  r->flag = UWILDREGEX_FLAG_VALID;
  r->nre = uwildsplit->argc;

  strsplit_delete(uwildsplit);

  return(status);
}

static void uwildregex_error(struct uwildregex_st *r, regex_t *reg){

  size_t size;
  char *errbuffer = NULL;

  size = regerror(r->errcode, reg, r->errbuffer, r->errbuffer_size);
  if(size > r->errbuffer_size){
    errbuffer = malloc(size);
    if(errbuffer != NULL){
      free(r->errbuffer);
      r->errbuffer_size = size;
      r->errbuffer = errbuffer;
      (void)regerror(r->errcode, reg, r->errbuffer, r->errbuffer_size);
    }
  }
}

int uwildregex_match(struct uwildregex_st *r, char *s){
  /*
   * Returns:
   *
   *   0 => null or the right-most matching pattern is not negated  (accept)
   *   1 => empty, or no match found, or the right-most matching pattern
   *        is negated     (reject)
   *  -1 => error
   *
   *   If the uwildregex is invalid it is treated as NULL.
   */
  int status = 1;	/* no match */
  int i;

  if((r->flag == UWILDREGEX_FLAG_NULL) || (r->flag == UWILDREGEX_FLAG_INVALID))
    return(0);
  else if(r->flag == UWILDREGEX_FLAG_EMPTY)
    return(1);

  for(i = 0; i < r->nre; ++i){
    r->errcode = regexec(&r->re[i], s, 0, NULL, 0);
    if(r->errcode == 0)
      status = r->negate_flag[i];
    else if((r->errcode != 0) && (r->errcode != REG_NOMATCH)){
      uwildregex_error(r, &r->re[i]);
      status = -1;
      break;
    }
  }

  return(status);
}

#if 0
/*
 * TEST
 */
#include <err.h>
#include <stdio.h>
int main(int argc, char **argv){

  char *s = NULL;
  char *uwildregex = NULL;
  struct uwildregex_st *r;
  int status = 0;

  if(argc < 3)
    errx(1, "Number of arguments");

  if(argv[1][0] != '-')
    uwildregex = argv[1];

  s = argv[2];

  r = uwildregex_create();
  if(r == NULL)
    err(1, "uwildregex_create()");

  status = uwildregex_init(r, uwildregex);
  if(status != 0)
    errx(1, r->errbuffer);

  status = uwildregex_match(r, s);
  fprintf(stdout, "%d\n", status);

  uwildregex_free(r);

  return(0);
}
#endif

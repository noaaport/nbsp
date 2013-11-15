/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id: strsplit.c,v bc2b0f5cffa5 2013/11/12 22:22:28 nieves $
 */
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "strsplit.h"

static struct strsplit_st *strsplit_init(int n, char *s);
static void strsplit(struct strsplit_st *strp, char *delim, int flags);

struct strsplit_st *strsplit_create(char *s, char *delim, int flags){

  return(strsplit_recreate(s, delim, flags, NULL));
}

struct strsplit_st *strsplit_recreate(char *s, char *delim, int flags,
				      struct strsplit_st *strp){
  struct strsplit_st *new_strp;
  char *p;
  int n;	/* number of fields found */
  int b;	/* number of characters accumulated in a field */

  if((s == NULL) || (*s == '\0'))
    return(NULL);

  /*
   * Count the number of fields.
   */
  n = 0;	
  p = s;
  b = 0;	
  while(*p != '\0'){
    if(strchr(delim, *p) != NULL){
      if((b != 0) || (flags == STRSPLIT_FLAG_INCEMPTY))
	++n;

      b = 0;
    }else
      ++b;

    ++p;
  }

  /*
   * The last field.
   */
  if((b != 0) || (flags == STRSPLIT_FLAG_INCEMPTY))
    ++n;

  if((strp != NULL) && (strp->argc >= n) && (strlen(strp->sp) >= strlen(s)))
    new_strp = strp;
  else{
    if(strp != NULL)
      strsplit_delete(strp);

    new_strp = strsplit_init(n, s);
  }

  strsplit(new_strp, delim, flags);

  return(new_strp);
}

void strsplit_delete(struct strsplit_st *strp){

  if(strp->sp != NULL){
    free(strp->sp);
    strp->sp = NULL;
  }

  if(strp->argv != NULL){
    free(strp->argv);
    strp->argv = NULL;
  }

  free(strp);
}

static struct strsplit_st *strsplit_init(int n, char *s){

  struct strsplit_st *strp = NULL;
  char *sp;
  size_t s_size;
  char **argv;

  s_size = strlen(s);
  sp = (char*)malloc(s_size + 1);
  if(sp == NULL)
    return(NULL);

  strncpy(sp, s, s_size + 1);
  
  argv = (char**)calloc(n, sizeof(char*));
  if(argv == NULL){
    free(sp);
    return(NULL);
  }

  strp = (struct strsplit_st*)malloc(sizeof(struct strsplit_st));
  if(strp == NULL){
    free(argv);
    free(sp);
    return(NULL);
  }

  strp->sp = sp;
  strp->argc = n;
  strp->argv = argv;

  return(strp);
}

static void strsplit(struct strsplit_st *strp, char *delim, int flags){

  char *nextp;
  char *p;
  int n = 0;

  nextp = strp->sp;
  p = strp->sp;
  do {
    p = strsep(&nextp, delim);
    if((*p != '\0') || (flags == STRSPLIT_FLAG_INCEMPTY))
      strp->argv[n++] = p;
  } while(nextp != NULL);

  assert(strp->argc == n);
}

#if 0
/*
 * Test
 */
#include <err.h>
#include <stdio.h>
static void strsplit_print(struct strsplit_st *strp);
static char buffer[128];
static int  buffer_size = 128;
int main(int argc, char **argv){

  int status = 0;
  struct strsplit_st *strp = NULL;
  int length;
  char *fname;
  FILE *f;

  if(argc == 1)
    errx(1, "Needs argument.");
  
  fname = argv[1];
  f = fopen(fname, "r");
  if(f == NULL)
    errx(1, "fopen");

  while(fgets(buffer, buffer_size, f) != NULL){
    length = strlen(buffer);
    if(buffer[length - 1] == '\n')
      buffer[length - 1] = '\0';

    strp = strsplit_recreate(buffer, ",", 0, strp);
    if(strp == NULL)
      err(1, "strplit_create()");
    
    strsplit_print(strp);
    
    /*    strsplit_delete(strp); */
  }
  strsplit_delete(strp);
  fclose(f);

  return(status);
}

static void strsplit_print(struct strsplit_st *strp){

  int i;

  for(i = 0; i < strp->argc; ++i)
    fprintf(stdout, "%s:", strp->argv[i]);

  fprintf(stdout, "\n");
}
#endif

/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdlib.h>
#include "err.h"
#include "nbspre.h"
#include "globals.h"
#include "const.h"

static int nbsp_regex_create(struct uwildregex_st **r, char *patt_accept);

static int nbsp_regex_create(struct uwildregex_st **r, char *patt_accept){

  struct uwildregex_st *rp;
  int status = 0;

  rp = uwildregex_create();
  if(rp == NULL){
    log_err("Error initalizing regex module.");
    return(-1);
  }

  status = uwildregex_init(rp, patt_accept);
  if(status != 0){
    log_errx("Error initalizing regex module: %s", rp->errbuffer);
    uwildregex_free(rp);
    rp = NULL;
  }

  *r = rp;

  return(status);
}

int init_nbsp_regex(void){

  int status = 0;

  status = nbsp_regex_create(&g.np_regex, g.patt_accept);

  if(status == 0)
    status = nbsp_regex_create(&g.filterq_regex, g.filterq_patt_accept);

  if(status == 0)
    status = nbsp_regex_create(&g.serverq_regex, g.serverq_patt_accept);

  if(status == 0)
    status = nbsp_regex_create(&g.nbs1_regex, g.nbs1_patt_accept);

  if(status == 0)
    status = nbsp_regex_create(&g.nbs2_regex, g.nbs2_patt_accept);

  if(status == 0)
    status = nbsp_regex_create(&g.emwin_regex, g.emwin_patt_accept);

  if(status == 0)
    status = nbsp_regex_create(&g.savez_regex, g.savez_patt_accept);

  if(status == 0)
    status = nbsp_regex_create(&g.rtxdb_regex, g.rtxdb_patt_accept);

  return(status);
}

void free_nbsp_regex(void){

  if(g.np_regex != NULL)
    uwildregex_free(g.np_regex);

  if(g.filterq_regex != NULL)
    uwildregex_free(g.filterq_regex);

  if(g.nbs1_regex != NULL)
    uwildregex_free(g.nbs1_regex);

  if(g.nbs2_regex != NULL)
    uwildregex_free(g.nbs2_regex);

  if(g.emwin_regex != NULL)
    uwildregex_free(g.emwin_regex);

  if(g.savez_regex != NULL)
    uwildregex_free(g.savez_regex);

  if(g.rtxdb_regex != NULL)
    uwildregex_free(g.rtxdb_regex);
}

int np_regex_match(char *name){

  return(uwildregex_match(g.np_regex, name));
}

int filterq_regex_match(char *name){

  return(uwildregex_match(g.filterq_regex, name));
}

int serverq_regex_match(char *name){

  return(uwildregex_match(g.serverq_regex, name));
}

int nbs1_regex_match(char *name){

  return(uwildregex_match(g.nbs1_regex, name));
}

int nbs2_regex_match(char *name){

  return(uwildregex_match(g.nbs2_regex, name));
}

int emwin_regex_match(char *name){

  return(uwildregex_match(g.emwin_regex, name));
}

int savez_regex_match(char *name){

  return(uwildregex_match(g.savez_regex, name));
}

int rtxdb_regex_match(char *name){

  return(uwildregex_match(g.rtxdb_regex, name));
}

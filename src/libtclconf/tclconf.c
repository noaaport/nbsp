/*
 * Copyright (c) 2004 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include "config.h"

#ifndef HAVE_ERR
void err(int eval, const char *fmt, ...);
void errx(int eval, const char *fmt, ...);
void warn(const char *fmt, ...);
void warnx(const char *fmt, ...);
#else
  #include <err.h>
#endif
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include "tclcreate.h"
#include "tclconf.h"

static int link_vars(Tcl_Interp *interp, struct confoption_st *opt);
static void unlink_vars(Tcl_Interp *interp, struct confoption_st *opt);
static int exists_configfile(char *fname);

static int link_vars(Tcl_Interp *interp, struct confoption_st *opt){

  struct confoption_st *op = opt;
  char *p;

  while(op->name != NULL){
    if(op->type == TCL_LINK_STRING)
      p = (char*)&(op->p);     /* It is assumed that op->p has been set NULL */
    else
      p = (char*)&(op->v);

    Tcl_LinkVar(interp, op->name, p, op->type);
    ++op;
  }

  return(TCL_OK);
}

static void unlink_vars(Tcl_Interp *interp, struct confoption_st *opt){

  struct confoption_st *op = opt;

  while(op->name != NULL){
    Tcl_UnlinkVar(interp, op->name);
    ++op;
  }
}

void kill_confopt_table(struct confoption_st *opt){

  struct confoption_st *op = opt;

  while(op->name != NULL){
    if((op->type == TCL_LINK_STRING) && (op->p != NULL)){
      ckfree(op->p);	/* use the Tcl memory function */
      op->p = NULL;
    }

    ++op;
  }
}

int parse_configfile(char *scriptname, struct confoption_st *opt){

  int status = TCL_OK;
  Tcl_Interp *interp;

  interp = tcl_create_interp();
  if(interp == NULL){
    warn("Could not create the tcl interpreter in tcl_create_interp().");
    return(-1);
  }

  status = link_vars(interp, opt);
  if(status != TCL_OK){
    warn("Error in link_vars().");
    tcl_delete_interp(interp);
    return(-1);
  }

  if(status == TCL_OK){
    status = Tcl_EvalFile(interp, scriptname);
    if(status != TCL_OK)
      warnx("while reading %s\n%s", scriptname, Tcl_GetStringResult(interp));
  }

  unlink_vars(interp, opt);
  tcl_delete_interp(interp);

  return(status);
}

int parse_configfile2(char *scriptname, struct confoption_st *opt){
  /*
   * Same as parse_configfile() but if the file does not exist it is not
   * an error. Just return.
   */
  int status = 0;

  status = exists_configfile(scriptname);
  if(status == 1)
    status = 0;
  else if(status == -1)
    warn("Cannot read %s.", scriptname);
  else
    status = parse_configfile(scriptname, opt);

  return(status);
}

struct confoption_st *find_confoption(struct confoption_st *opt, int id){

  struct confoption_st *op = opt;

  while(op->name != NULL){
    if(op->id == id)
      return(op);

    ++op;
  }

  return(NULL);
}

static int exists_configfile(char *fname){
  /* 
   * Returns:
   * 0 == > ok. file exists
   * 1 ==> file does not exist
   * -1 ==> error from systat different from ENOENT
   */
  struct stat stbuf;
  int status = 0;

  if(stat(fname,&stbuf) == -1){
    if(errno == ENOENT)
      status = 1;
    else
      status = -1;
  }

  return(status);
}

void setoptval(void *var, struct confoption_st *optable, int id){
  /*
   * This a utility function for applications. var should be a pointer
   * to a string, when the option is a string value, or a pointer to an
   * int when the option is an integer.
   */
  struct confoption_st *op;

  op = find_confoption(optable, id);

  /*
   *  assert(op != NULL);
   */

  if(op->type == TCL_LINK_STRING){
    if(op->p != NULL)
      *(char**)var = op->p;
  }else 
    *(int*)var = op->v;
}

Tcl_Interp *tcl_create_interp(void){

  Tcl_Interp *interp;

  interp = Tcl_CreateInterp();
  if(interp == NULL)
    return(NULL);

  if(Tcl_Init(interp) == TCL_ERROR){
    Tcl_DeleteInterp(interp);
    return(NULL);
  }

  return(interp);
}

void tcl_delete_interp(Tcl_Interp *interp){

  Tcl_DeleteInterp(interp);
}

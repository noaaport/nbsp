/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <tcl.h>
#include "libtclconf/tclcreate.h"
#include "err.h"
#include "tclevalfile.h"

/*
 * These functions were introduced to evaluate, once, the scripts
 * to start/stop external programs (e.g., innd).
 * See the comment in tcleval.h.
 */

int tcl_eval_file(char *script, char *input){

  int status = TCL_OK;
  Tcl_Interp *interp;

  interp = tcl_create_interp();
  if(interp == NULL){
    log_err2("Cannot evaluate", script);
    return(-1);
  }

  if(input != NULL){
    Tcl_LinkVar(interp, "input", (void*)&input, TCL_LINK_STRING);
  }

  status = Tcl_EvalFile(interp, script);
  if(status != TCL_OK)
    log_errx("Error reading %s.\n%s", script, Tcl_GetStringResult(interp));

  tcl_delete_interp(interp);

  return(status);
}

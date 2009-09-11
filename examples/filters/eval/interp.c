#include <stdio.h>
#include <err.h>
#include <tcl.h>

char *emwinfname = NULL;

int init(Tcl_Interp *interp){

  Tcl_LinkVar(interp, "emwinfname", (void*)&emwinfname, TCL_LINK_STRING);

  return(0);
}

int main(int argc, char **argv){

  char *script;
  int status = TCL_OK;
  Tcl_Interp *interp;
  
  if(argc == 1)
    errx(1, "Need one argument.");

  script = argv[1];

  interp = Tcl_CreateInterp();
  if(interp == NULL){
    warn("Cannot configure.");
    return(1);
  }

  status = init(interp);
  if(status == TCL_OK){
    status = Tcl_EvalFile(interp, script);
    if(status != TCL_OK)
      warnx("While reading %s\n%s", script, Tcl_GetStringResult(interp));
  }

  Tcl_DeleteInterp(interp);

  if(emwinfname != NULL)
    fprintf(stdout, "emwinfname = %s\n", emwinfname);
  else
    fprintf(stdout, "emwinfname = NULL\n");

  return(0);
}

/*
 * setrlimit.c
 * The limit Tcl command.
 */

#include <tcl.h>
#include <sys/time.h>
#include <sys/resource.h>


static int  LimitCmd _ANSI_ARGS_((ClientData clientData,
            Tcl_Interp *interp, int argc, char *argv[]));

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT

EXTERN int  Limit_Init _ANSI_ARGS_((Tcl_Interp *interp));


/*
 * LimitCmd --
 *  Set resource limits.
 *
 * Results:
 *  none
 *
 * Side Effects:
 *  Set resource limits.
 */
int
LimitCmd(ClientData data, Tcl_Interp *interp, int argc, char *argv[])
{
    int max;
    char buf[32];
    struct rlimit limit;
    Tcl_ResetResult(interp);
    if (getrlimit(RLIMIT_NOFILE, &limit) < 0) {
    Tcl_AppendResult(interp, "NOFILE: ", Tcl_PosixError(interp), NULL);
    return TCL_ERROR;
    }
    if (argc > 1) {
    Tcl_GetInt(interp, argv[1], (int *)&limit.rlim_cur);
        if (setrlimit(RLIMIT_NOFILE, &limit) < 0) {
        Tcl_AppendResult(interp, "NOFILE: ", Tcl_PosixError(interp), NULL);
        return TCL_ERROR;
    }
    }
    /* bad, bad style, direct writing to interp->result */
    sprintf(interp->result, "%d %d", limit.rlim_cur, limit.rlim_max);
    return TCL_OK;
}

/*
 * Limit_Init --
 *  Initialize the Tcl limit facility.
 *
 * Results:
 *  TCL_OK.
 *
 * Side Effects:
 *  None.
 */
int
Limit_Init(Tcl_Interp *interp)
{
    int code;
    #ifdef USE_TCL_STUBS
        if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
            return TCL_ERROR;
        }
    #endif

    Tcl_CreateCommand(interp, "limit", LimitCmd, NULL, NULL);
    code = Tcl_PkgProvide(interp, "limit", "1.0");
    if (code != TCL_OK) {
    return code;
    }
    return TCL_OK;
}

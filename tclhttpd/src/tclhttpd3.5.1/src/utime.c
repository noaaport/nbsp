/*
 * setuid.c
 * The setuid Tcl command.
 */

#include <tcl.h>
#include <sys/types.h>
#include <utime.h>

/*
 * UtimeCmd --
 *	Change file access and modification times.
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	Change access and modification times.
 */
int
UtimeCmd(ClientData data, Tcl_Interp *interp, int argc, char *argv[])
{
    struct utimbuf u = {0, 0};

    Tcl_ResetResult(interp);
    if (argc > 3) {
	Tcl_GetInt(interp, argv[3], &u.actime);	/* new access time */
    }
    if (argc > 2) {
	Tcl_GetInt(interp, argv[2], &u.modtime);	/* new modify time */
    }
    if (argc > 4 || argc <= 1) {
	Tcl_AppendResult(interp, "Usage: utime file ?modtime? ?actime?", NULL);
	return TCL_ERROR;
    }
    if (utime(argv[1], &u) < 0) {
	Tcl_AppendResult(interp, "utime: ", argv[1], Tcl_PosixError(interp), NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 * Utime_Init --
 *	Initialize the Tcl utime facility.
 *
 * Results:
 *	TCL_OK.
 *
 * Side Effects:
 *	None.
 */
int
Utime_Init(Tcl_Interp *interp)
{
    Tcl_CreateCommand(interp, "utime", UtimeCmd, NULL, NULL);
    Tcl_PkgProvide(interp, "utime", "1.0");
    return TCL_OK;
}

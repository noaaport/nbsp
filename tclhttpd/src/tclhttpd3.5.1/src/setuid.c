/*
 * setuid.c
 * The setuid Tcl command.
 */

#include <tcl.h>
#include <sys/types.h>

/*
 * SetuidCmd --
 *	Change effective user id.
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	Change effective user id.
 */
int
SetuidCmd(ClientData data, Tcl_Interp *interp, int argc, char *argv[])
{
    int uid;
    Tcl_ResetResult(interp);
    if (argc > 2) {
	Tcl_GetInt(interp, argv[2], &uid);	/* new group id */
	if (setgid((uid_t)uid) < 0) {
	    Tcl_AppendResult(interp, "setgid: ", Tcl_PosixError(interp), NULL);
	    return TCL_ERROR;
	}
    }
    uid = 60001;
    if (argc > 1) {
	Tcl_GetInt(interp, argv[1], &uid);	/* new user id */
    }
    if (setuid((uid_t)uid) < 0) {
	Tcl_AppendResult(interp, "setuid: ", Tcl_PosixError(interp), NULL);
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 * Setuid_Init --
 *	Initialize the Tcl setuid facility.
 *
 * Results:
 *	TCL_OK.
 *
 * Side Effects:
 *	None.
 */
int
Setuid_Init(Tcl_Interp *interp)
{
    Tcl_CreateCommand(interp, "setuid", SetuidCmd, NULL, NULL);
    Tcl_PkgProvide(interp, "setuid", "1.0");
    return TCL_OK;
}

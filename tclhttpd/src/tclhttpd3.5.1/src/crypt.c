/* 
 * crypt.c --
 *
 *	This file contains a simple Tcl package "crypt" that is a
 *	thin layer over the crypt C library.
 *
 * Copyright (c) 1997 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include "tcl.h"
#ifndef _WIN32
#include "unistd.h"
#endif

/*
 * Prototypes for procedures defined later in this file:
 */


static int	Crypt_Cmd _ANSI_ARGS_((ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv));

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT

EXTERN int	Crypt_Init _ANSI_ARGS_((Tcl_Interp *interp));


/*
 *----------------------------------------------------------------------
 *
 * Crypt_Cmd --
 *
 *	This procedure is invoked to process the "crypt" Tcl command.
 *	It expects two arguments, the password and a two character "salt".
 *	It returns the encrypted password.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

static int
Crypt_Cmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    char *result;
    if (argc != 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" password salt\"", (char *) NULL);
	return TCL_ERROR;
    }
    result = (char *)crypt(argv[1], argv[2]);
    if (result == NULL) {
	/* an error has occurred */
	Tcl_SetResult(interp, "crypt error", TCL_STATIC);
	return TCL_ERROR;
    } else {
	Tcl_SetResult(interp, result, TCL_STATIC);
	return TCL_OK;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Crypt_Init --
 *
 *	This is a package initialization procedure, which is called
 *	by Tcl when this package is to be added to an interpreter.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

int Crypt_Init(interp)
    Tcl_Interp *interp;		/* Interpreter in which the package is
				 * to be made available. */
{
    int code;

#ifdef USE_TCL_STUBS
	if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
	    return TCL_ERROR;
	}
#endif

    code = Tcl_PkgProvide(interp, "crypt", "1.0");
    if (code != TCL_OK) {
	return code;
    }
    Tcl_CreateCommand(interp, "crypt", Crypt_Cmd, (ClientData) 0,
	    (Tcl_CmdDeleteProc *) NULL);
    return TCL_OK;
}

/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef TCL_CREATE_H
#define TCL_CREATE_H

#include <tcl.h>

Tcl_Interp *tcl_create_interp(void);
void tcl_delete_interp(Tcl_Interp *interp);

#endif

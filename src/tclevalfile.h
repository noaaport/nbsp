/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef TCLEVALFILE_H
#define TCLEVALFILE_H

/*
 * These functions were introduced to evaluate, once, the scripts
 * to start/stop external programs (e.g., innd). Although tcl_eval_file()
 * can be used to evaluate a script anytime, it should not be used
 * to do so continously since it creates an interpreter every time
 * it is called with full access to the global environment, which
 * can lead to corruption or just undesired modications of global variables. 
 */
int tcl_eval_file(char *script, char *input);

#endif

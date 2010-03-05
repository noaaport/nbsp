/*
 * Copyright (c) 2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef CONST_H
#define CONST_H

/*
 * "common.h" has copies of various constants that used to be defined
 * in src/const.h and src/sbn.h.
 */
#include "common.h"

/*
 * These are constants used only by the programs not nbspd.
 */
#define ZBYTE0			(unsigned int)120
#define ZBYTE1			(unsigned int)218

#define GMPK_HEADER_FMT		"\001\r\r\n%03d \r\r\n"
#define GMPK_TRAILER_STR	"\r\r\n\003"
#define GMPK_HEADER_SIZE	11

#endif

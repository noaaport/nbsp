/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCNIDS_NAME_H
#define DCNIDS_NAME_H

#include "dcnids_header.h"

#define DCNIDS_DEFAULT_TIME_FMT  "%d%02d%02d_%02d%02d"
#define DCNIDS_DEFAULT_NAME_FMT  "%s_" DCNIDS_DEFAULT_TIME_FMT

#define DCNIDS_PNGEXT	".png"
#define DCNIDS_SHPEXT   ".shp"
#define DCNIDS_SHXEXT   ".shx"
#define DCNIDS_DBFEXT   ".dbf"
#define DCNIDS_INFOEXT  ".info"
#define DCNIDS_CSVEXT   ".csv"

char *dcnids_default_name(struct nids_header_st *nheader,
			  char *prefix,
			  char *suffix);
char *dcnids_optional_name(char *base, char *suffix);

#endif

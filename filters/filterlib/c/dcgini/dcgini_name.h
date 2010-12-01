/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef DCGINI_NAME_H
#define DCGINI_NAME_H

#include "dcgini_pdb.h"

#define DCGINI_DEFAULT_TIME_FMT  "%d%02d%02d_%02d%02d"
#define DCGINI_DEFAULT_NAME_FMT  "%s_" DCGINI_DEFAULT_TIME_FMT

#define DCGINI_PNGEXT	".png"
#define DCGINI_SHPEXT   ".shp"
#define DCGINI_SHXEXT   ".shx"
#define DCGINI_DBFEXT   ".dbf"
#define DCGINI_INFOEXT  ".info"
#define DCGINI_CSVEXT   ".csv"

char *dcgini_default_name(struct nesdis_pdb_st *npdb,
			  char *prefix,
			  char *suffix);
char *dcgini_optional_name(char *base, char *suffix);

#endif

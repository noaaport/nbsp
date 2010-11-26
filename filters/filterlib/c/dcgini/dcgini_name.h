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

#define DCGINI_DEFAULT_NAME_FMT  "%s_%d%02d%02d_%02d%02d"
#define DCGINI_TIME_STR_SIZE	 13  /* yyyymmdd_hhmm */
#define DCGINI_BASENAME_STR_SIZE 14  /* _yyyymmdd_hhmm */

#define DCGINI_PNGEXT	".png"
#define DCGINI_SHPEXT   ".shp"
#define DCGINI_SHXEXT   ".shx"
#define DCGINI_DBFEXT   ".dbf"
#define DCGINI_INFOEXT  ".info"
#define DCGINI_CSVEXT   ".csv"

#define DCGINI_FNAME_SIZE        20	  /* e.g. tigp04_yyyymmdd_hhmm */
#define DCGINI_GEMPAK_FNAME_SIZE DCGINI_FNAME_SIZE
#define DCGINI_PNG_FNAME_SIZE	 24	  /* e.g. tigp04_yyyymmdd_hhmm.png */

char *dcgini_default_name(struct nesdis_pdb *npdb,
			  char *prefix,
			  char *suffix);

#endif






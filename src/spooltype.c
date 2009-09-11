/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include "const.h"
#include "spooltype.h"
#include "globals.h"

int spooltype_fsspool(void){

  if(g.spooltype == SPOOLTYPE_FS)
    return(1);

  return(0);
}

int spooltype_cspool(void){

  if((g.spooltype == SPOOLTYPE_CBDB) || (g.spooltype == SPOOLTYPE_MCBDB))
    return(1);

  return(0);  
}

int spooltype_mspool(void){

  if(g.spooltype == SPOOLTYPE_MBDB)
    return(1);

  return(0);  
}

int spooltype_cspool_nofile(void){

  if(g.spooltype == SPOOLTYPE_MCBDB)
    return(1);

  return(0);  
}

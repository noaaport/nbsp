/*
 * Copyright (c) 2005-2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include "err.h"
#include "dcgini_pdb.h"
#include "dcgini_misc.h"

int dcgini_verify_wmo_header(void *header){
  /*
   * Check the first NESDIS_WMO_HEADER_SIZE bytes to see if it consists of
   * 18 ascii bytes plus the \r\r\n.
   */
  int i;
  char *c;

  c = (char*)header;
  for(i = 1; i <= NESDIS_WMO_HEADER_SIZE - 3; ++i){    
    if(isascii(*c++) == 0)
      goto end;
  }

  if((*c++ == '\r') && (*c++ == '\r') && (*c == '\n'))
    return(0);

 end:

  return(1);
}

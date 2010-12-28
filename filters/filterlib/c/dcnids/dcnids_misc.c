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
#include "const.h"
#include "err.h"
#include "dcnids_misc.h"

static int dcnids_verify_wmo_header(void *header);
static int dcnids_verify_awips_header(void *header);

int dcnids_verify_wmoawips_header(void *header){

  if((dcnids_verify_wmo_header(header) == 0) &&
     (dcnids_verify_awips_header(header) == 0))
    return(0);

  return(1);
}

static int dcnids_verify_wmo_header(void *header){
  /*
   * Check the first CTRLHDR_WMO_SIZE bytes to see if it consists of
   * 18 ascii bytes plus the \r\r\n.
   */
  int i;
  char *c;

  c = (char*)header;
  for(i = 1; i <= CTRLHDR_WMO_SIZE - 3; ++i){    
    if(isprint(*c++) == 0)
      goto end;
  }

  if((*c++ == '\r') && (*c++ == '\r') && (*c == '\n'))
    return(0);

 end:

  return(1);
}

static int dcnids_verify_awips_header(void *header){
  /*
   * Check the last WMO_AWIPS_SIZE + 3 bytes to see if it consists of
   * 6 ascii bytes plus the \r\r\n.
   */
  int i;
  char *c;

  c = (char*)header;
  c += CTRLHDR_WMO_SIZE;
  for(i = 1; i <= WMO_AWIPS_SIZE; ++i){
    if(isprint(*c++) == 0)
      goto end;
  }

  if((*c++ == '\r') && (*c++ == '\r') && (*c == '\n'))
    return(0);

 end:

  return(1);
}

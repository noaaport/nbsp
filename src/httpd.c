/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include "err.h"
#include "util.h"
#include "globals.h"
#include "tclhttpd.h"
#include "httpd.h"

#define TCLHTTPD_FORK

int spawn_httpd_server(void){

  int status = 0;

#ifdef TCLHTTPD_FORK
  g.httpd = tclhttpd_open(g.tclhttpd, g.tclhttpd_fifo);
#else
  g.httpd = tclhttpd_open(g.tclhttpd);
#endif

  if(g.httpd == NULL){
    log_err("Error creating the httpd server.");
    status = -1;
  }else
    log_info("Spawned the http server.");

  return(status);
}

void kill_httpd_server(void){

  if(g.httpd == NULL)
    return;

  tclhttpd_close(g.httpd);
  g.httpd = NULL;
}

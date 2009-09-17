/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "err.h"
#include "util.h"
#include "globals.h"

static int httpd_open(void);
static void httpd_close(void);

int spawn_httpd_server(void){

  return(httpd_open());
}

void kill_httpd_server(void){

  httpd_close();
}

static int httpd_open(void){

  if(valid_str(g.httpd) == 0){
    log_errx("No httpd defined.");
    return(1);
  }

  g.httpdfp = popen(g.httpd, "w");
  if(g.httpdfp != NULL){
    if(fprintf(g.httpdfp, "%s\n", "init") < 0){
      (void)pclose(g.httpdfp);
      g.httpdfp = NULL;
    }
  }

  if(g.httpdfp == NULL){
    log_err("Could not start httpd server.");
    return(-1);
  }else
    log_info("Started httpd.");

  return(0);
}

static void httpd_close(void){

  if(g.httpdfp == NULL)
    return;

  if(pclose(g.httpdfp) == -1)
    log_err("Error closing httpd server.");
  else
    log_info("Stoped httpd.");

  g.httpdfp = NULL;
}

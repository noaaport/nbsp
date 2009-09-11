/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include "../nbspq.h"
#include "../globals.h"
#include "../err.h"
#include "../qstate.h"
#include "../nbspre.h"
#include "framep.h"

/*
 * This is the function called by the reading routine to process the
 * the fpaths received. In this type of slave there is no pctl processor
 * since the file is already saved in the spool diretory. So this
 * function only mimicks the functionality of the last stage of the nbsp
 * processor which is to insert the packetinfo in the server and filter
 * queues.
 */
int slavefpproc(struct packet_info_st *packetinfo){

  int status1 = 0;
  int status2 = 0;

  if(filterq_regex_match(packetinfo->fname) == 0)
    status1 = e_nbspq_snd_filter(packetinfo->packet, packetinfo->packet_size);
  else
    log_verbose(1, "Filter queue regex is rejecting %s.",
		packetinfo->fname);

  if(serverq_regex_match(packetinfo->fname) == 0)
    status2 = e_nbspq_snd_server(packetinfo->packet, packetinfo->packet_size);
  else
    log_verbose(1, "Server queue regex is rejecting %s.",
		packetinfo->fname);
    
  if(status1 == 0)
    status1 = status2;

  /*
   * Since the processor is not running in this mode, the periodic
   * function of the processor is not invoked. We must do whatever is
   * required here.
   */
  if(status1 == 0)
    e_report_qstate();
  
  return(status1);
}

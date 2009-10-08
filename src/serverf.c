/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

/*
 * This file the functions to support the netfilter in the server.
 */
#include <assert.h>
#include <sys/param.h>
#include <stdlib.h>
#include <string.h>
#include "libconnth/libconn.h"
#include "defaults.h"
#include "sfilter.h"
#include "server_priv.h"

/*
 * We will use Tcl_GetVar2 and Tcl_SetVar2 which use separate arguments
 * for the array name and the index. Since the arguments are scalar variables,
 * then in contrast to Tcl_GetVar and Tcl_SetVar, they do not modify the
 * arguments and we can leave them as static strings.
 */
#define ALLOW_VARNAME			"allow"
#define ALLOW_DEFAULT_INDEX		"0"
#define SERVERINFO_VARNAME		"serverinfo"
#define SERVERINFO_CLIENTNAME_INDEX	"clientname"
#define SERVERINFO_CLIENTIP_INDEX	"clientip"
#define SERVERINFO_CLIENT_EMPTYVAL	""

/*
 * The server sets the serverinfo() variables, that the filter can use.
 * The filter must set the the variables
 *
 * allow(192.168.2.7)
 * allow(noaaport.uprrp.edu)
 *
 * and we here try to get their values or the default allow(0).
 */

struct allow_filter_st {
  int allow;	/* the result of the allow variable for each client */
};

static void free_filterdata(void *p);
static void allow_filter_set_flag(struct conn_element_st *ce, int flag);
static int tcl_get_allow_val(Tcl_Interp *interp, char *index, int *val);
static int tcl_set_serverinfo(Tcl_Interp *interp, char *index, char *val);

static int tcl_get_allow_val(Tcl_Interp *interp, char *index, int *val){
  /*
   * allow_index should the hostname or hostip of the client. This function
   * returns the value of allow(<hostname>) or allow(<hostip>). 
   * 
   * Returns
   *
   * 0 if the variable could be fetched.
   * 1 otherwise
   */
  const char *result;
  int status = 0;

  result = Tcl_GetVar2(interp, ALLOW_VARNAME, index, 0);
  if(result == NULL)
    status = 1;

  if(status == 0){
    if(Tcl_GetInt(interp, result, val) != TCL_OK)
      status = 1;
  }

  return(status);
}

static int tcl_set_serverinfo(Tcl_Interp *interp, char *index, char *val){
  /*
   * index is either "clientname" or "clientip" and
   * val should the hostname or hostip of the client,
   * respectively.
   * 
   * Returns
   *
   * 0 if the variable could be set.
   * 1 otherwise
   */
  const char *result;
  int status = 0;

  result = Tcl_SetVar2(interp, SERVERINFO_VARNAME, index, val, 0);
  if(result == NULL)
    status = 1;

  return(status);
}

int exec_net_filter(struct sfilterp_st *sfilterp, struct conn_table_st *ct){
  /*
   * The job of this function is to Tcl_EvalFile the netfilter
   * and fillup the allow variable for each ce in the table using the 
   * variable's values defined by the rc file of the filter.
   */
  char *name;
  char *ip;
  int numentries;
  int isnetclient;
  int f_thread_created;
  int f_thread_finished;
  int allow_hardcoded_default = NETFILTER_ALLOW_DEFAULT;
  int allow_filter_default;
  int allow_result;
  int status = 0;
  int i;

  numentries = conn_table_get_numentries(ct);
  for(i = 0; i < numentries; ++i){
    f_thread_created = conn_table_get_element_fthread_created(ct, i);
    f_thread_finished =  conn_table_get_element_fthread_finished(ct, i);
    isnetclient = conn_element_isclient(&ct->ce[i]);

    if((isnetclient == 0) || (f_thread_created == 0) || f_thread_finished)
      continue;

    name = conn_element_get_name(&ct->ce[i]);
    ip = conn_element_get_ip(&ct->ce[i]);

    if(name != NULL)
      status = tcl_set_serverinfo(sfilterp->interp,
				  SERVERINFO_CLIENTNAME_INDEX, name);
    else
      status = tcl_set_serverinfo(sfilterp->interp,
				  SERVERINFO_CLIENTNAME_INDEX,
				  SERVERINFO_CLIENT_EMPTYVAL);

    if(status == 0){
      if(ip != NULL)
	status = tcl_set_serverinfo(sfilterp->interp,
				    SERVERINFO_CLIENTIP_INDEX, ip);
      else
	status = tcl_set_serverinfo(sfilterp->interp,
				    SERVERINFO_CLIENTIP_INDEX,
				    SERVERINFO_CLIENT_EMPTYVAL);
    }

    if(status == 0){
      if(exec_sfilter_script(sfilterp) != 0)
	status = -1;
    }

    /*
     * Any error up to this point is considered fatal, in the sense that
     * the filter cannot be properly evaluated.
     */
    if(status != 0)
      break;

    /*
     * Here we try to retrieve the allow setting for each client, and
     * errors are not considered fatal.
     */
    status = tcl_get_allow_val(sfilterp->interp,
			       ALLOW_DEFAULT_INDEX, &allow_filter_default);
    if(status != 0){
      allow_filter_default = allow_hardcoded_default;
      status = 0;
    }
    
    if(name != NULL)
      status = tcl_get_allow_val(sfilterp->interp, name, &allow_result);

    if((name == NULL) || (status != 0))
      status = tcl_get_allow_val(sfilterp->interp, ip, &allow_result);

    if(status != 0){
      allow_result = allow_filter_default;
      status = 0;
    }

    allow_filter_set_flag(&ct->ce[i], allow_result);
  }

  return(status);
}

int allow_filter_init(struct conn_element_st *ce){

  struct allow_filter_st *allowp;
  int status = 0;

  allowp = (struct allow_filter_st*)malloc(sizeof(struct allow_filter_st));
  if(allowp == NULL)
    return(-1);

  allowp->allow = NETFILTER_ALLOW_DEFAULT;

  status = conn_element_init3(ce, (void*)allowp, free_filterdata);
  if(status != 0){
    free(allowp);
    return(-1);
  }

  return(0);
}

static void allow_filter_set_flag(struct conn_element_st *ce, int flag){

  struct allow_filter_st *allowp = (struct allow_filter_st*)ce->filterdata;

  assert(allowp != NULL);
  
  allowp->allow = flag;
}

int allow_filter_get_flag(struct conn_element_st *ce){

  struct allow_filter_st *allowp = (struct allow_filter_st*)ce->filterdata;

  assert(allowp != NULL);

  return(allowp->allow);
}

static void free_filterdata(void *p){

  assert(p != NULL);

  free(p);
}

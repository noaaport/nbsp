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

#define VAR_DEFAULT	"allow(0)"
#define VAR_FMT		"allow(%s)"
#define VAR_ADD_SIZE	7		/* strlen("allow()") */
static char *s_var_default = NULL;	/* see allocation below */

/*
 * var_ip and var_name are the variables that the filter must
 * set, and whose values we will try to get here.
 */
struct allow_filter_st {
  int allow;
  char *var_ip;		/* e.g., allow(192.168.2.7) */
  char *var_name;	/* e.g., allow(noaaport.uprrp.edu) */
};

static void free_filterdata(void *p);
static int tcl_get_int(Tcl_Interp *interp, char *varname, int *val);

static void allow_filter_set_flag(struct conn_element_st *ce, int flag);
static char *allow_filter_get_varip(struct conn_element_st *ce);
static char *allow_filter_get_varname(struct conn_element_st *ce);

static int tcl_get_int(Tcl_Interp *interp, char *varname, int *val){
  /*
   * Returns
   *
   * 1 if the variable could not be fetched.
   * 0 otherwise
   */
  const char *result;
  int status = 0;

  result = Tcl_GetVar(interp, varname, 0);
  if(result == NULL)
    status = 1;

  if(status == 0){
    if(Tcl_GetInt(interp, result, val) != TCL_OK)
      status = 1;
  }

  return(status);
}
  
int exec_net_filter(struct sfilterp_st *sfilterp, struct conn_table_st *ct){
  /*
   * The job of this function is to Tcl_EvalFile the netfilter
   * and fillup the allow variable for each ce in the table using the 
   * variable's values defined by the rc file of the filter.
   */
  char *var_name;
  char *var_ip;
  int numentries;
  int isnetclient;
  int f_thread_created;
  int f_thread_finished;
  int allow_hardcoded_default = NETFILTER_ALLOW_DEFAULT;
  int allow_filter_default;
  int allow_result;
  int status = 0;
  int i;

  status = exec_sfilter_script(sfilterp);
  if(status != 0)
    return(-1);

  /*
   * Use a dynamically allocated variable for the "allow(0)" name
   * to work around the resrtiction of Tcl_GetVar that it modifies the
   * variable when it is an array name.
   */

  /* First time */
  if(s_var_default == NULL){
    s_var_default = malloc(strlen(VAR_DEFAULT) + 1);
    if(s_var_default != NULL)
      strncpy(s_var_default, VAR_DEFAULT, strlen(VAR_DEFAULT) + 1);
  }

  if(s_var_default != NULL)
    status = tcl_get_int(sfilterp->interp, s_var_default,
			 &allow_filter_default);

  if((status != 0) || (s_var_default == NULL)){
    allow_filter_default = allow_hardcoded_default;
    status = 0;
  }

  numentries = conn_table_get_numentries(ct);
  for(i = 0; i < numentries; ++i){
    f_thread_created = conn_table_get_element_fthread_created(ct, i);
    f_thread_finished =  conn_table_get_element_fthread_finished(ct, i);
    isnetclient = conn_element_isclient(&ct->ce[i]);

    if((isnetclient == 0) || (f_thread_created == 0) || f_thread_finished)
      continue;

    var_ip = allow_filter_get_varip(&ct->ce[i]);
    var_name = allow_filter_get_varname(&ct->ce[i]);

    if(var_name != NULL)
      status = tcl_get_int(sfilterp->interp, var_name, &allow_result);

    if((var_name == NULL) || (status != 0))
      status = tcl_get_int(sfilterp->interp, var_ip, &allow_result);

    if(status != 0)
      allow_result = allow_filter_default;

    allow_filter_set_flag(&ct->ce[i], allow_result);
  }

  return(0);
}

int allow_filter_init(struct conn_element_st *ce){

  struct allow_filter_st *allowp;
  char *var_ip = NULL;
  char *var_name = NULL;
  char *ip;
  char *name;
  int status = 0;

  allowp = (struct allow_filter_st*)malloc(sizeof(struct allow_filter_st));
  if(allowp == NULL)
    return(-1);

  ip = conn_element_get_ip(ce);
  name = conn_element_get_name(ce);

  var_ip = malloc(strlen(ip) + VAR_ADD_SIZE + 1);
  if(var_ip == NULL){
    free(allowp);
    return(-1);
  }
  snprintf(var_ip, strlen(ip) + VAR_ADD_SIZE + 1, VAR_FMT, ip);

  if(name != NULL){
    var_name = malloc(strlen(name) + VAR_ADD_SIZE + 1);
    if(var_name == NULL){
      free(var_ip);
      free(allowp);
      return(-1);
    }
    snprintf(var_name, strlen(name) + VAR_ADD_SIZE + 1, VAR_FMT, name);
  }

  allowp->allow = NETFILTER_ALLOW_DEFAULT;
  allowp->var_ip = var_ip;
  allowp->var_name = var_name;

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

static char *allow_filter_get_varip(struct conn_element_st *ce){

  struct allow_filter_st *allowp = (struct allow_filter_st*)ce->filterdata;

  assert(allowp != NULL);

  return(allowp->var_ip);
}

static char *allow_filter_get_varname(struct conn_element_st *ce){

  struct allow_filter_st *allowp = (struct allow_filter_st*)ce->filterdata;

  assert(allowp != NULL);

  return(allowp->var_name);
}

static void free_filterdata(void *p){

  struct allow_filter_st *allowp = (struct allow_filter_st*)p;

  assert(p != NULL);

  if(allowp->var_ip != NULL)
    free(allowp->var_ip);

  if(allowp->var_name != NULL)
    free(allowp->var_name);

  free(p);
}

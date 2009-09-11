/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "err.h"
#include "libtclconf/tclcreate.h"
#include "sfilter.h"

#define INIT_INPUT_SIZE	1

struct sfilterp_st *open_sfilter(char *script){

  struct sfilterp_st *sfilterp;
  int status = 0;

  sfilterp = malloc(sizeof(struct sfilterp_st));
  if(sfilterp == NULL)
    return(NULL);

  /* 
   * The variables output_xxxx are set by the filter and read by the server.
   * The other ones are set by the server and read by the filter.
   */
  sfilterp->script = NULL;
  sfilterp->tcl_scriptpath = NULL;
  sfilterp->input = NULL;
  sfilterp->fdata = NULL;
  sfilterp->input_size = 0;
  sfilterp->fdata_size = 0;
  sfilterp->cmd = SFILTER_CMD_INIT;
  sfilterp->output_status = 0;		/* set by the filter */
  sfilterp->output_fpathout = NULL;	/* set by the filter */
  sfilterp->output_emwinfname = NULL;	/* set by the filter */

  if(set_sfilter_script(sfilterp, script) != 0){
    free(sfilterp);
    return(NULL);
  }

  sfilterp->interp = tcl_create_interp();
  if(sfilterp->interp == NULL)
    status = -1;

  if(status == 0){
    sfilterp->input = ckalloc(INIT_INPUT_SIZE);
    if(sfilterp->input == NULL)
      status = -1;
    else{
      sfilterp->input[0] = '\0';
      sfilterp->input_size = INIT_INPUT_SIZE;
    }
  }

  if(status == 0){
    Tcl_LinkVar(sfilterp->interp, "input",
		(void*)&sfilterp->input, TCL_LINK_STRING);
    Tcl_LinkVar(sfilterp->interp, "fdata",
	      (void*)&sfilterp->fdata, TCL_LINK_STRING);
    Tcl_LinkVar(sfilterp->interp, "fdata_size",
	      (void*)&sfilterp->fdata_size, TCL_LINK_INT);
    Tcl_LinkVar(sfilterp->interp, "command",
		(void*)&sfilterp->cmd, TCL_LINK_INT);
    Tcl_LinkVar(sfilterp->interp, "output_status",
		(void*)&sfilterp->output_status, TCL_LINK_INT);
    Tcl_LinkVar(sfilterp->interp, "output_fpathout",
		(void*)&sfilterp->output_fpathout, TCL_LINK_STRING);
    Tcl_LinkVar(sfilterp->interp, "output_emwinfname",
		(void*)&sfilterp->output_emwinfname, TCL_LINK_STRING);
  }

  if(status == 0){
    status = pthread_mutex_init(&sfilterp->mutex, NULL);
    if(status == 0){
      status = pthread_cond_init(&sfilterp->cond, NULL);
      if(status != 0)
	pthread_mutex_destroy(&sfilterp->mutex);
    }
    if(status != 0){
      errno = status;
      status = -1;
    }else
      sfilterp->thread_status = 0;
  }

  if(status != 0){
    close_sfilter(sfilterp);
    sfilterp = NULL;
  }

  return(sfilterp);
}

void close_sfilter(struct sfilterp_st *sfilterp){
  /*
   * The sfilterp->fdata is not created nor freed by us in this module
   * (see init_filter_fdata()). In addition, the
   * variables output_fpathout and output_emwinfname "belong" to the
   * interpreter (free with ckfree()).
   */  
  assert(sfilterp != NULL);

  /*
   * (NetBSD complains about an ambigous `else' without the braces.)
   */
  if(sfilterp->tcl_scriptpath != NULL){
    Tcl_DecrRefCount(sfilterp->tcl_scriptpath);
  }

  if(sfilterp->script != NULL)
    ckfree(sfilterp->script);

  if(sfilterp->input != NULL){
    ckfree(sfilterp->input);
    sfilterp->input = NULL;
  }

  if(sfilterp->output_fpathout != NULL){
    ckfree(sfilterp->output_fpathout);
    sfilterp->output_fpathout = NULL;
  }

  if(sfilterp->output_emwinfname != NULL){
    ckfree(sfilterp->output_emwinfname);
    sfilterp->output_emwinfname = NULL;
  }

  if(sfilterp->interp != NULL)
    tcl_delete_interp(sfilterp->interp);

  free(sfilterp);
}

int init_sfilter_input(struct sfilterp_st *sfilterp,
		       struct packet_info_st *packetinfo){

  int n;
  char *p;
  int size = sfilterp->input_size;

  n = snprintf(sfilterp->input, size, "%u %d %d %d %d %s %s",
	       (unsigned int)packetinfo->seq_number, 
	       packetinfo->psh_product_type,
	       packetinfo->psh_product_category,
	       packetinfo->psh_product_code,
	       packetinfo->np_channel_index,
	       packetinfo->fname,
	       packetinfo->fpath);
 
  /* n does not include the trailing '\0' */
  if(n >= size){
    while(n >= size){
      size *= 2;
    }

    p = ckrealloc(sfilterp->input, size);
    if(p == NULL)
      return(-1);

    sfilterp->input = p;
    sfilterp->input_size = size;

    n = snprintf(sfilterp->input, size, "%u %d %d %d %d %s %s",
		 (unsigned int)packetinfo->seq_number, 
		 packetinfo->psh_product_type,
		 packetinfo->psh_product_category,
		 packetinfo->psh_product_code,
		 packetinfo->np_channel_index,
		 packetinfo->fname,
		 packetinfo->fpath);
  }

  return(0);
}

int init_sfilter_fdata(struct sfilterp_st *sfilterp,
		       void *fdata, size_t fdata_size){

  sfilterp->fdata = fdata;
  sfilterp->fdata_size = fdata_size;

  return(0);
}

int set_sfilter_script(struct sfilterp_st *sfilterp, char *script){

  int status = 0;
  char *s;
  int length;

  assert((script != NULL) && (script[0] != '\0'));

  length = strlen(script);
  s = ckalloc(length + 1);
  if(s == NULL)
    return(-1);

  /*
   * cc (GCC) 4.2.1 20070719  [FreeBSD] was complaining with:
   * warning: suggest explicit braces to avoid ambiguous 'else'
   */
  if(sfilterp->tcl_scriptpath != NULL){
    Tcl_DecrRefCount(sfilterp->tcl_scriptpath);
  }

  strncpy(s, script, length + 1);
  sfilterp->script = s;

  sfilterp->tcl_scriptpath = Tcl_NewStringObj(s, -1);
  if(sfilterp->tcl_scriptpath == NULL){
    ckfree(s);
    sfilterp->script = NULL;
    return(-1);
  }else{
    Tcl_IncrRefCount(sfilterp->tcl_scriptpath);
  }

  return(status);
}

int exec_sfilter_script(struct sfilterp_st *sfilterp){

  int status = 0;
  int cmd = SFILTER_CMD_EXEC;

  sfilterp->cmd = cmd;
  status = exec_sfilter_cmd(sfilterp);

  return(status);
}

int init_sfilter_script(struct sfilterp_st *sfilterp){

  int status = 0;
  int cmd = SFILTER_CMD_INIT;

  sfilterp->cmd = cmd;
  status = exec_sfilter_cmd(sfilterp);
  if(status == 0){
    if(sfilterp->output_status != 0){
      log_errx("Error initializing %s.", sfilterp->script);
      status = 1;
    }
  }

  return(status);
}

int end_sfilter_script(struct sfilterp_st *sfilterp){

  int status = 0;
  int cmd = SFILTER_CMD_END;

  sfilterp->cmd = cmd;
  status = exec_sfilter_cmd(sfilterp);
  if(status == 0){
    if(sfilterp->output_status != 0){
      log_errx("Error terminating %s.", sfilterp->script);
      status = 1;
    }
  }

  return(status);
}

int exec_sfilter_cmd(struct sfilterp_st *sfilterp){

  int status = 0;

  /*
   * Initialize (in case of an error in the script).
   */
  sfilterp->output_status = 1;

  /*
  status = Tcl_EvalFile(sfilterp->interp, sfilterp->script);
  */

  status = Tcl_FSEvalFile(sfilterp->interp, sfilterp->tcl_scriptpath);

  if(status != TCL_OK){
    status = 1;
    log_errx("Error executing %s:\n%s", sfilterp->script,
	     Tcl_GetStringResult(sfilterp->interp));
  }

  return(status);
}

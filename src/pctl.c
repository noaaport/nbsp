/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
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
#include "pctl.h"

static int pctl_init(struct pctl_st *pctl, int numchannels,
		     struct pctldb_param_st *param, int *dberror);

static struct pctl_element_st **create_pce_array(int numchannels,
						 size_t memfile_size);
static void destroy_pce_array(struct pctl_element_st **pcearray,
			      int numchannels);

static void pctldb_log_err(char *s, int dberror);

static void pctldb_log_err(char *s, int dberror){

  if(dberror > 0){
    log_err(s);
  }else if(dberror < 0){
    log_errx("%s %s", s, db_strerror(dberror));
  }
}

/*
 * Create/destroy the pctl
 */
struct pctl_st *epctl_open(int numchannels, struct pctldb_param_st *param){

  int status = 0;
  int dberror;
  struct pctl_st *pctl = NULL;

  pctl = malloc(sizeof(struct pctl_st));
  if(pctl != NULL){
    status = pctl_init(pctl, numchannels, param, &dberror);
    if(status != 0){
      pctldb_log_err("Cannot open pctl queue.", dberror);      
      free(pctl);
      pctl = NULL;
    }
  }

  return(pctl);
}

static int pctl_init(struct pctl_st *pctl, int numchannels,
		     struct pctldb_param_st *param, int *dberror){
  int status = 0;
  int dberror_close;
  struct pctldb_st *pctldb = NULL;

  pctl->pctldb = NULL;
  pctl->activepce = NULL;
  pctl->outputmf = NULL;
  pctl->pce = NULL;
  pctl->status.current = 0;
  pctl->status.last = 0;

  status = pctldb_open(&pctldb, param, dberror);
  if(status != 0)
    return(status);

  pctl->activepce = create_pce_array(numchannels, param->memfile_minsize);
  if(pctl->activepce == NULL){
    *dberror = errno;
    status = -1;
    goto end;
  }
  pctl->num_activechannels = numchannels;

  pctl->outputmf = create_memfile(param->memfile_minsize);
  if(pctl->outputmf == NULL){
    *dberror = errno;
    status = -1;
    goto end;
  }

  pctl->pce = create_pce(param->memfile_minsize);
  if(pctl->pce == NULL){
    *dberror = errno;
    status = -1;
    goto end;
  }

 end:

  if(status != 0){
    if(pctldb != NULL)
      (void)pctldb_close(pctldb, &dberror_close);  /* error ignored here */

    if(pctl->activepce != NULL)
      destroy_pce_array(pctl->activepce, pctl->num_activechannels);

    if(pctl->outputmf != NULL)      
      close_memfile(pctl->outputmf);

    if(pctl->pce != NULL)
      destroy_pce(pctl->pce);
  }else
    pctl->pctldb = pctldb;

  return(status);
}

void epctl_close(struct pctl_st *pctl){

  int status = 0;
  int dberror;

  assert(pctl != NULL);

  if(pctl == NULL)
    return;

  if(pctl->pctldb != NULL){
    status = pctldb_close(pctl->pctldb, &dberror);
    pctl->pctldb = NULL;
    if(status != 0)
      pctldb_log_err("Could not close pctl queue.", dberror);
  }

  if(pctl->activepce != NULL){
    destroy_pce_array(pctl->activepce, pctl->num_activechannels);
    pctl->activepce = NULL;
  }
  
  if(pctl->outputmf != NULL){
    destroy_memfile(pctl->outputmf);
    pctl->outputmf = NULL;
  }

  if(pctl->pce != NULL){
    destroy_pce(pctl->pce);
    pctl->pce = NULL;
  }
}

int epctl_send(struct pctl_st *pctl, int channel_index){
  /*
   * Sends to the pctldb queue the pce of the active channel.
   */
  int status = 0;
  int dberror;
  int i;

  assert((channel_index >= 0) && (channel_index < pctl->num_activechannels));
  if((channel_index < 0) || (channel_index >= pctl->num_activechannels))
    return(-1);

  i = channel_index;

  status = pctldb_send_pce(pctl->pctldb, pctl->activepce[i], &dberror);
  pctl->status.current = status;
  if(status == 1){
    log_verbose(2, "Soft size-limit reached for pctldb.");
    status = 0;
  }else if(status == 2){
    log_errx("Rejecting additions to pctldb. Hard size-limit reached.");
  }else if(status != 0)
    pctldb_log_err("Could not add element to pctldb.", dberror);

  return(status);  
}

int epctl_rcv(struct pctl_st *pctl, int timeout_ms){
  /*
   * This function waits for the pctldb to be ready and dequeues
   * an element an copies it to the working pce.
   *
   * This function returns 
   *
   *  0 if there are no errors reading.
   * -1 if there were errors
   *  1 if the queue was empty (after the timeout)
   *
   * It logs an error message if it returns -1, but not in the other case.
   */
  int status = 0;
  int dberror;

  status = pctldb_rcv_pce(pctl->pctldb, pctl->pce, timeout_ms, &dberror);
  if(status == -1)
    pctldb_log_err("Cannot retrieve element from pctldb.", dberror);

  return(status);
}

int epctl_sync(struct pctl_st *pctl){

  int dberror;
  int status = 0;

  status = pctldb_sync(pctl->pctldb, &dberror);
  if(status == -1)
    pctldb_log_err("Cannot sync pctldb.", dberror);
  else
    log_info("Synczed pctlmf.");

  return(status);
}

void epctl_report_status(struct pctl_st *pctl){
  /*
   * Check the limits, and report only if:
   * (1) the quota is exceeded.
   * (2) an overflow was restored.
   */

  if(pctl->status.current == 1)
    log_warnx("Soft size-limit reached for pctldb.");
  else if(pctl->status.current == 2)
    log_warnx("Hard size-limit reached for pctldb.");
  else if((pctl->status.current == 0) && 
	  (pctl->status.current != pctl->status.last))
    log_warnx("Size-limit restored for pctldb.");

  pctl->status.last = pctl->status.current;
}

int esend_pctl_activepce(struct pctl_st *pctl,
			      int channel_index){
  int status;

  status = epctl_send(pctl, channel_index);

  return(status);
}

struct pctl_element_st *eopen_pctl_outputpce(struct pctl_st *pctl,
					    int timeout_ms,
					    int *status){
  struct pctl_element_st *pce = NULL;

  *status = epctl_rcv(pctl, timeout_ms);
  if(*status == 0)
    pce = get_pctl_outputpce(pctl);

  return(pce);
}

/*
 * Access to non-shared elements of pctl
 */
struct memfile_st *reopen_pctl_outputmf(struct pctl_st *pctl){
  /*
   * The processor reuses the memfile (stored in the pctl) for
   * assembling the frames. The memfile is initialized when the
   * pctl is initialized, and this function just reopens it.
   */  
  struct memfile_st *mf = pctl->outputmf;

  close_memfile(mf);
  if(open_memfile(mf) != 0)
     return(NULL);

  return(mf);
}

void close_pctl_outputpce(struct pctl_st *pctl){

  close_pce(pctl->pce);
}

struct pctl_element_st *get_pctl_outputpce(struct pctl_st *pctl){

  return(pctl->pce);
}

struct pctl_element_st *open_pctl_activepce(struct pctl_st *pctl,
					    int channel_index){

  assert((channel_index >= 0) && (channel_index < pctl->num_activechannels));
  if((channel_index < 0) || (channel_index >= pctl->num_activechannels))
    return(NULL);

  /*
   * Returning NULL lets the caller know that there is an unfinished
   * product in the active pce.
   */
  if(open_pce(pctl->activepce[channel_index]) != 0)
    return(NULL);

  return(pctl->activepce[channel_index]);
}

void close_pctl_activepce(struct pctl_st *pctl, int channel_index){

  assert((channel_index >= 0) && (channel_index < pctl->num_activechannels));
  if((channel_index < 0) || (channel_index >= pctl->num_activechannels))
    return;

  close_pce(pctl->activepce[channel_index]);
}

struct pctl_element_st *get_pctl_activepce(struct pctl_st *pctl,
					   int channel_index,
					   unsigned int seqnumber){

  int i = channel_index;

  assert((channel_index >= 0) && (channel_index < pctl->num_activechannels));

  if((channel_index < 0) || (channel_index >= pctl->num_activechannels))
    return(NULL);

  if((pctl->activepce[i])->seq_number != seqnumber)
    return(NULL);

  return(pctl->activepce[i]);
}

int update_activepce_framecounter(struct pctl_st *pctl,
				  int channel_index,
				  unsigned int seqnumber,
				  int block_number){
  int i;
  int status = 0;

  assert((channel_index >= 0) && (channel_index < pctl->num_activechannels));

  if((channel_index < 0) || (channel_index >= pctl->num_activechannels))
    return(-1);

  i = channel_index;
  if((pctl->activepce[i])->seq_number != seqnumber)
    status = 1;
  else{
    if(block_number == (pctl->activepce[i])->recv_fragments){
      ++(pctl->activepce[i])->recv_fragments;
    }else{
      /*
       * At least one frame in the sequence has been missed.
       */
      status = 2;
    }
  }

  return(status);
}

int complete_activepce(struct pctl_st *pctl, int channel_index,
		       unsigned int seqnumber){
  /*
   * The frame processor (readers) call this when received the END
   * transfer code. Check that the sequence number of the active
   * pce matches and that all the frames have been received.
   */ 
  int i;
  int status = 0;

  assert((channel_index >= 0) && (channel_index < pctl->num_activechannels));

  if((channel_index < 0) || (channel_index >= pctl->num_activechannels))
    return(-1);

  i = channel_index;
  if((pctl->activepce[i])->seq_number != seqnumber)
    status = 1;
  else{
    if((pctl->activepce[i])->num_fragments > 
       (pctl->activepce[i])->recv_fragments){
      /*
       * At least one frame in the sequence has been missed.
       */
      status = 2;
    }else
      (pctl->activepce[i])->f_complete = 1;
  }

  return(status);
}
 
/*
 * Create and destroy a pce
 */
int const_pce_size(void){

  int pce_size;

  /* const_fpath_maxsize() does not include '\0' */
  pce_size = sizeof(struct pctl_element_st) + const_fpath_maxsize() + 1;
  
  return(pce_size);
}

int const_pce_data_size(void){

  int pce_data_size;
  /*
   * In the functions that copy one pce to another, this saves some
   * bytes in memmcpy(). For the case in which fpath is declard as fpath[1],
   * it is the same as the pce_size.
   *
   * pce_data_size = sizeof(struct pctl_element_st) - PATH_MAX + 
   * const_fpath_maxsize();
   */

   pce_data_size = const_pce_size();

  return(pce_data_size);
}

struct pctl_element_st *create_pce(size_t memfile_size){

  struct pctl_element_st *pce;
  int pce_size;

  pce_size = const_pce_size();
  pce = malloc(pce_size);
  if(pce == NULL)
    return(NULL);

  pce->mf = create_memfile(memfile_size);
  if(pce->mf == NULL){
    free(pce);
    return(NULL);
  }

  pce->seq_number = 0;
  pce->fpath_size = const_fpath_maxsize() + 1;

  return(pce);
}

void destroy_pce(struct pctl_element_st *pce){

  assert(pce != NULL);

  if(pce == NULL)
    return;

  close_pce(pce);

  if(pce->mf != NULL)
    destroy_memfile(pce->mf);

  free(pce);
}

int open_pce(struct pctl_element_st *pce){
  
  int status = 1;

  if(pce->seq_number == 0)
    status = open_memfile(pce->mf);

  return(status);
}

void close_pce(struct pctl_element_st *pce){

  assert(pce != NULL);

  if(pce == NULL)
    return;

  if(pce->mf != NULL)
    close_memfile(pce->mf);

  pce->seq_number = 0;  
}
  
static struct pctl_element_st **create_pce_array(int numchannels,
						 size_t memfile_size){
  int i;
  int status = 0;
  struct pctl_element_st **pcearray = NULL;

  pcearray = calloc(numchannels, sizeof(struct pctl_element_st*));
  if(pcearray == NULL)
    return(NULL);

  for(i = 0; i <= numchannels - 1; ++i)
    pcearray[i] = NULL;

  for(i = 0; i <= numchannels - 1; ++i){
    pcearray[i] = create_pce(memfile_size);
    if(pcearray[i] == NULL){
      status = -1;
      break;
    }
  }

  if(status != 0){
    destroy_pce_array(pcearray, numchannels);
    pcearray = NULL;
  }

  return(pcearray);
}

static void destroy_pce_array(struct pctl_element_st **pcearray,
			      int numchannels){
  int i;

  assert(pcearray != NULL);

  for(i = 0; i <= numchannels - 1; ++i){
    if(pcearray[i] != NULL)
      destroy_pce(pcearray[i]);
  }

  free(pcearray);
}

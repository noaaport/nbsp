/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SFILTER_H
#define SFILTER_H

#include <pthread.h>
#include <tcl.h>
#include "packfp.h"

#define SFILTER_CMD_INIT	0
#define SFILTER_CMD_EXEC	1
#define SFILTER_CMD_END		2

struct sfilterp_st {
  Tcl_Interp *interp;
  pthread_cond_t  cond;
  pthread_mutex_t mutex;
  int thread_status;
  char *script;
  Tcl_Obj *tcl_scriptpath;
  char *input;
  int input_size;
  void *fdata;
  size_t fdata_size;
  int cmd;		/* command to send to filter */
  int output_status;
  char *output_fpathout;
  char *output_emwinfname;
};

struct sfilterp_st *open_sfilter(char *script);
void close_sfilter(struct sfilterp_st *sfilterp);
int init_sfilter_input(struct sfilterp_st *sfilterp, struct packet_info_st *p);
int init_sfilter_fdata(struct sfilterp_st *sfilterp,
		       void *fdata, size_t fdata_size);
int set_sfilter_script(struct sfilterp_st *sfilterp, char *script);
int init_sfilter_script(struct sfilterp_st *sfilterp);
int exec_sfilter_script(struct sfilterp_st *sfilterp);
int end_sfilter_script(struct sfilterp_st *sfilterp);
int exec_sfilter_cmd(struct sfilterp_st *sfilterp);

#endif

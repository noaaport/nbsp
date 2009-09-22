/*
 * Copyright (c) 2009 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>	/* PRIuMAX */
#include "err.h"
#include "strsplit.h"
#include "defaults.h"	/* NPCAST_NUM_CHANNELS */
#include "stoi.h"
#include "slave.h"
#include "slavet.h"

static void slave_element_init(struct slave_element_st *slave);
static int slave_element_configure(struct slave_element_st *slave,
				   char *s,
				   struct slave_options_st *defaults);
static int slave_element_configure2(struct slave_element_st *slave,
				   int slavetype,
				   char *mastername,
				   char *masterport,
				   char *infifo,
				   struct slave_options_st *defaults);
static void slave_element_release(struct slave_element_st *slave);
static int slave_element_configure_options(struct slave_element_st *slave,
					   struct slave_options_st *options);
static void slave_element_release_options(struct slave_element_st *slave);

static int slave_table_alloc(struct slave_table_st **slavet, int numslaves);
static void slave_table_free(struct slave_table_st *slavet);

/*
 * Definitions
 */
static void slave_element_init(struct slave_element_st *slave){

  /*
   * Initialize the dynamic variables
   */
  slave->f_slave_thread_created = 0;
  /* slave->slave_thread_id */
  slave->slave_fd = -1;
  slave->slavenbs_reader_index = -1;
  slave->info = NULL;
  slave_stats_init(slave);
}

static int slave_element_configure(struct slave_element_st *slave,
				   char *s,
				   struct slave_options_st *defaults){
  int status = 0;
  struct strsplit_st *ssplit;
  int slavetype;
  char *mastername;
  char *masterport;
  char *infifo;
    
  ssplit = strsplit_create(s, SLAVE_STRING_SEP2, STRSPLIT_FLAG_INCEMPTY);
  if(ssplit == NULL){
    warn("strsplit_create()");
    return(-1);
  }

  mastername = NULL;
  masterport = NULL;
  infifo =  NULL;
  
  if(strto_int(ssplit->argv[0], &slavetype) != 0){
    log_errx("Invalid value for slavetype: %s", ssplit->argv[0]);
    status = 1;
    goto End;
  }

  if(slavetype == SLAVETYPE_INFIFO){
    if(ssplit->argc != 2){
      log_errx("Invalid configuration string for SLAVETYPE_INFIFO: %s", s);
      status = 1;
      goto End;
    }
    infifo =  ssplit->argv[1];
  } else {
    if(ssplit->argc != 3){
      log_errx("Invalid configuration string for SLAVETYPE_NBS: %s", s);
      status = 1;
      goto End;
    }
    mastername = ssplit->argv[1];
    masterport = ssplit->argv[2];
  }

  status = slave_element_configure2(slave,
				    slavetype,
				    mastername,
				    masterport,
				    infifo,
				    defaults);
 End:
  
  strsplit_delete(ssplit);

  return(status);
}


static int slave_element_configure2(struct slave_element_st *slave,
				    int slavetype,
				    char *mastername,
				    char *masterport,
				    char *infifo,
				    struct slave_options_st *defaults){

  if(slavetype == SLAVETYPE_INFIFO){
    assert(infifo != NULL);
    if(infifo == NULL){
      log_errx("infifo canot be NULL.");
      return(1);
    }

    if((slave->infifo = malloc(strlen(infifo) + 1)) == NULL){
      warn("Cannot allocate memory for slave->infifo.");
      return(-1);
    }

    slave->slavetype = slavetype;
    slave->mastername = NULL;
    slave->masterport = NULL;
    strncpy(slave->infifo, infifo, strlen(infifo) + 1);
  } else if((slavetype == SLAVETYPE_NBS1) || (slavetype == SLAVETYPE_NBS2)){
    assert(mastername != NULL);
    assert(masterport != NULL);
    if((mastername == NULL) || (masterport == NULL))
      return(1);

    if((slave->mastername = malloc(strlen(mastername) + 1)) == NULL){
      warn("Cannot allocate memory for slave->mastername");
      return(-1);
    }

    if((slave->masterport = malloc(strlen(masterport) + 1)) == NULL){
      warn("Cannot allocate memory for slave->masterport.");
      free(slave->mastername);
      return(-1);
    }

    slave->slavetype = slavetype;
    strncpy(slave->mastername, mastername, strlen(mastername) + 1);
    strncpy(slave->masterport, masterport, strlen(masterport) + 1);
    slave->infifo = NULL;
  } else {
    log_errx("Invalid value of slavetype: %d.", slavetype);
    return(1);
  }

  if(slave_element_configure_options(slave, defaults) != 0){
    warn("Cannot allocate memory for slave->options.");
    slave_element_release(slave);
    return(-1);
  }

  return(0);
}

static void slave_element_release(struct slave_element_st *slave){
  
  if(slave->mastername != NULL)
    free(slave->mastername);

  if(slave->masterport != NULL)
    free(slave->masterport);

  if(slave->infifo != NULL)
    free(slave->infifo);

  slave_element_release_options(slave);
}

static int slave_element_configure_options(struct slave_element_st *slave,
					   struct slave_options_st *options){

  slave->options.infifo_grp = malloc(strlen(options->infifo_grp) + 1);
  if(slave->options.infifo_grp == NULL)
    return(-1);

  slave->options.slavestatsfile = malloc(strlen(options->slavestatsfile) + 1);
  if(slave->options.slavestatsfile == NULL){
    free(slave->options.infifo_grp);
    return(-1);
  }

  slave->options.infifo_mode = options->infifo_mode;
  strncpy(slave->options.infifo_grp, options->infifo_grp,
	  strlen(options->infifo_grp) + 1);
  strncpy(slave->options.slavestatsfile, options->slavestatsfile,
	  strlen(options->slavestatsfile) + 1);
  slave->options.slave_read_timeout_s = options->slave_read_timeout_s;
  slave->options.slave_read_timeout_retry = 
    options->slave_read_timeout_retry;
  slave->options.slave_reopen_timeout_s = options->slave_reopen_timeout_s;
  slave->options.slave_so_rcvbuf = options->slave_so_rcvbuf;
  slave->options.slave_stats_logperiod_secs =
    options->slave_stats_logperiod_secs;

  return(0);
}

static void slave_element_release_options(struct slave_element_st *slave){

  if(slave->options.infifo_grp != NULL){
    free(slave->options.infifo_grp);
    slave->options.infifo_grp = NULL;
  }

  if(slave->options.slavestatsfile != NULL){
    free(slave->options.slavestatsfile);
    slave->options.slavestatsfile = NULL;
  }
}

static int slave_table_alloc(struct slave_table_st **slavet, int numslaves){

  int status = 0;
  struct slave_table_st *slavetp = NULL;
  struct slave_element_st *slave;

  assert(numslaves > 0);
  if(numslaves <= 0){
    log_errx("Invalid value of numslaves.");
    return(1);
  }

  slavetp = malloc(sizeof(struct slave_table_st));
  if(slavetp == NULL){
    warn("Cannot allocate memory for slavetp."); 
    return(-1);
  }

  slave = calloc(numslaves, sizeof(struct slave_element_st));
  if(slave == NULL){
    warn("Cannot allocate memory for slave."); 
    free(slavetp);
    return(-1);
  }

  if(status != 0){
    free(slave);
    free(slavetp);
    return(status);
  }

  slavetp->slave = slave;
  slavetp->numslaves = numslaves;
  slavetp->num_slavenbs_readers = 0;
  *slavet = slavetp;

  return(0);
}

static void slave_table_free(struct slave_table_st *slavet){

  if(slavet->slave != NULL)
    free(slavet->slave);

  free(slavet);
}

int slave_table_create(struct slave_table_st **slavet,
			  char *s,
			  struct slave_options_st *defaults){
  int status = 0;
  struct slave_table_st *slavetp = NULL;
  struct strsplit_st *ssplit;
  int i;

  ssplit = strsplit_create(s, SLAVE_STRING_SEP1, STRSPLIT_FLAG_INCEMPTY);
  if(ssplit == NULL){
    warn("Error from strsplit_create().");
    return(-1);
  }

  /* ssplit->argc is the number of slaves to create */
  status = slave_table_alloc(&slavetp, ssplit->argc);
  if(status != 0)
    goto End;

  for(i = 0; i < slavetp->numslaves; ++i){
    slave_element_init(&slavetp->slave[i]);
    status = slave_element_configure(&slavetp->slave[i],
				     ssplit->argv[i],
				     defaults);
    if(status != 0)
      goto End;
  }

  /*
   * Count the number of nbs readers (nbs1), and also define the index
   * each of them must use in the pctl table. If the master mode is enabled,
   * then the first four channels are used by the noaaport readers.
   * (see init_pctl() in nbsp.c).
   */
  slavetp->num_slavenbs_readers = 0;
  for(i = 0; i < slavetp->numslaves; ++i){
    if(slavetp->slave[i].slavetype == SLAVETYPE_NBS1){
      slavetp->slave[i].slavenbs_reader_index = slavetp->num_slavenbs_readers;
      if(feedmode_noaaport_enabled())
	slavetp->slave[i].slavenbs_reader_index += NPCAST_NUM_CHANNELS;

      ++slavetp->num_slavenbs_readers;
    }
  }

 End:

  strsplit_delete(ssplit);
  if(status != 0){
    if(slavetp != NULL)
      slave_table_destroy(slavetp);
  } else {
    *slavet = slavetp;    
  }

  return(status);
}

void slave_table_destroy(struct slave_table_st *slavet){

  int i;

  /*
   * Release the resources used by each slave.
   */
  for(i = 0; i < slavet->numslaves; ++i)
    slave_element_release(&slavet->slave[i]);

  slave_table_free(slavet);
}

/*
 * slave->stats functions
 */
void slave_stats_init(struct slave_element_st *slave){

  slave->stats.connect_errors = 0;
  slave->stats.ctime = 0;
  slave->stats.errors_ctime = 0;
  slave->stats.packets_ctime = 0;
  slave->stats.bytes_ctime = 0.0;
  slave->stats.rtime = 0;
  slave->stats.errors_rtime = 0;
  slave->stats.packets_rtime = 0;
  slave->stats.bytes_rtime = 0.0;
}

void slave_stats_connect(struct slave_element_st *slave){

  slave->stats.ctime = time(NULL);
  slave->stats.errors_ctime = 0;
  slave->stats.packets_ctime = 0;
  slave->stats.bytes_ctime = 0.0;
}

void slave_stats_reset(struct slave_element_st *slave){

  slave->stats.rtime = time(NULL);
  slave->stats.errors_rtime = 0;
  slave->stats.packets_rtime = 0;
  slave->stats.bytes_rtime = 0.0;
}

void slave_stats_update_packets(struct slave_element_st *slave,
				size_t packet_size){

  ++slave->stats.packets_rtime;
  slave->stats.bytes_rtime += (double)packet_size;

  ++slave->stats.packets_ctime;
  slave->stats.bytes_ctime += (double)packet_size;
}

void slave_stats_update_errors(struct slave_element_st *slave){

  ++slave->stats.errors_rtime;
  ++slave->stats.errors_ctime;
}

void slave_stats_update_connect_errors(struct slave_element_st *slave){

  ++slave->stats.connect_errors;
}

void slave_stats_report(struct slave_element_st *slave){
  /*
   * This is a utility function for the slave threads.
   * If the threads use the same file for logging, they must synchronize
   * the access, or leave it to fopen().
   * This function is called by slave_stats_update_packets().
   */
  time_t now;
  FILE *f;

  now = time(NULL);
  if(now < slave->stats.rtime + 
     (time_t)slave->options.slave_stats_logperiod_secs){
    return;
  }

  if(slave->options.slavestatsfile == NULL)
    return;

  f = fopen(slave->options.slavestatsfile, "a");
  if(f == NULL){
    log_err_open(slave->options.slavestatsfile);
    return;
  }

  /*  now = time(NULL); */
  fprintf(f, "%s %s" " %" PRIuMAX " %u" " %" PRIuMAX " %u %u %.1g"
	  " %" PRIuMAX " %u %u %.1g\n",
	  slave->mastername, slave->masterport,
	  (uintmax_t)now,
	  slave->stats.connect_errors,
	  (uintmax_t)slave->stats.ctime,
	  slave->stats.errors_ctime,
	  slave->stats.packets_ctime,
	  slave->stats.bytes_ctime,
	  (uintmax_t)slave->stats.rtime,
	  slave->stats.errors_rtime,
	  slave->stats.packets_rtime,
	  slave->stats.bytes_rtime);

  fclose(f);
  slave_stats_reset(slave);
}

/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "readn.h"
#include "globals.h"
#include "err.h"
#include "nbspq.h"
#include "qstate.h"

static int greopen_qstatefifo_flag = 0;

static int e_open_qstate_fifo(void);
static int e_reopen_qstate_fifo(void);
static void e_close_qstate_fifo(void);
static int create_qstatefifo_buffer(void);
static int qstatefifo_buffer_size(void);
static int get_reopen_qstatefifo_flag(void);

int init_qstate_fifo(void){
  /*
   * Creates the fifo to report the internal state of the queues.
   */
  int status = 0;

  unlink(g.qstatefifo);
  status  = mkfifo(g.qstatefifo, g.qstatefifo_mode);

  if(status == 0)
     status = create_qstatefifo_buffer();

  if(status != 0)
    log_err2("Could not create", g.qstatefifo);

  return(status);
}

void kill_qstate_fifo(void){

  if(g.qstatefifo_buffer != NULL){
    free(g.qstatefifo_buffer);
    g.qstatefifo_buffer = NULL;
  }

  e_close_qstate_fifo();

  unlink(g.qstatefifo);
}

void e_report_qstate(void){
  /*
   * Report the state of the queue table and the pctl table.
   */
  int status = 0;
  int fd;
  uint32_t *buffer = NULL;
  int size;
  int i, j;
  int n;

  if(get_reopen_qstatefifo_flag())
    (void)e_reopen_qstate_fifo();

  fd = g.qstatefifo_fd;  
  if(fd == -1)
    return;

  /*
   * The number of channels in the queue is g.queue_numchannels, and
   * we report that first. Each queue channel has two integers (n, nmax)
   * and the same for the pctl. This is arranged in the function 
   * create_state_fifo_buffer().
   */
  size = g.qstatefifo_buffer_size;
  buffer = (uint32_t*)g.qstatefifo_buffer;

  j = 0;
  buffer[j++] = g.queue_numchannels;
  buffer[j++] = (g.pctl->pctldb)->n;
  for(i = 0; i <= g.queue_numchannels - 1; ++i){
    if(g.qtable != NULL)
      buffer[j++] = (g.qtable->nbspq[i])->n;
    else
      buffer[j++] = 0;
  }

  buffer[j++] = (g.pctl->pctldb)->nmax;
  for(i = 0; i <= g.queue_numchannels - 1; ++i){
    if(g.qtable != NULL)
      buffer[j++] = (g.qtable->nbspq[i])->nmax;
    else
      buffer[j++] = 0;
  }

  n = writem(fd, buffer, size, g.fifo_write_timeout_ms, 0);
  if(n == -1){
    if(errno == EPIPE)
      status = 1;
    else
      status = -1;
  }else if(n < size)
    status = -2;

  if(status == 1){
    /*
     * fifo closed on the other side.
     */
    e_close_qstate_fifo();
  }
  
  if(status == -1)
    log_err_write(g.qstatefifo);
  else if(status == -2)
    log_errx("Timed out writing to %s.", g.qstatefifo);
}

static int e_open_qstate_fifo(void){
  /*
   * Try to open the fifo to report the internal queue state.
   * Returns:
   *  0 if there are no errors, or
   *    if the file does not exist or is not opened for reading (at other end)
   * -1 for real open errors. It writes the appropriate message.
   */
  int fd;
  int status = 0;

  assert(g.qstatefifo != NULL);
  
  /*
   * open write only but non block
   */
  fd = open(g.qstatefifo, O_WRONLY | O_NONBLOCK, 0);
  if(fd == -1){
    if((errno == ENOENT) || (errno == ENXIO))
      status = 0;
    else
      status = -1;
  }else
    g.qstatefifo_fd = fd;

  if(status == -1)
    log_err_open(g.qstatefifo);

  return(status);
}

static void e_close_qstate_fifo(void){

  int status = 0;

  if(g.qstatefifo_fd != -1){
    status = close(g.qstatefifo_fd);
    g.qstatefifo_fd = -1;
  }

  if(status != 0)
    log_err2("Could not close", g.qstatefifo);
}

static int e_reopen_qstate_fifo(void){

  int fd;
  int status = 0;

  fd = g.qstatefifo_fd;
  if(fd == -1)
    status = e_open_qstate_fifo();

  return(status);
}

static int create_qstatefifo_buffer(void){
  /*
   * The number of channels in the queue is g.queue_numchannels, and
   * we report that first. Each queue channel has two integers (n, nmax)
   * and the same for the pctl.
   */
  int size;
  int status = 0;

  size = qstatefifo_buffer_size();
  g.qstatefifo_buffer = malloc(size);
  if(g.qstatefifo_buffer == NULL){
    status = -1;
  }else
    g.qstatefifo_buffer_size = size;

  return(status);
}

static int qstatefifo_buffer_size(void){
  /*
   * The number of channels in the queue is g.queue_numchannels, and
   * we report that first. Each queue channel has two integers (n, nmax)
   * and the same for the pctl.
   */
  int size;

  size = sizeof(uint32_t) * 2 * (g.queue_numchannels + 1) + sizeof(uint32_t);

  return(size);
}

void set_reopen_qstatefifo_flag(void){

  greopen_qstatefifo_flag = 1;
}

static int get_reopen_qstatefifo_flag(void){

  int flag;

  flag = greopen_qstatefifo_flag;
  if(flag == 1)
    greopen_qstatefifo_flag = 0;

  return(flag);
}

void log_qstate(void){
  /*
   * This reports esentially the same information about the queues,
   * but it writes it to a log file. In addition it calls e_nbspq_dbstats()
   * to write the output of dbstats (to another log file). It is meant
   * to be called periodically. It is called from per.c.
   */
  FILE *f = NULL;
  int i, n;
  time_t now;

  /* The output from dbstats (nbspq.c) */
  e_nbspq_dbstats();
  
  now = time(NULL);

  f = fopen(g.qstatelogfile, "a");
  if(f == NULL){
    log_err2("Error opening", g.qstatelogfile);
    return;
  }

  fprintf(f, "%" PRIuMAX " %" PRIu32, (uintmax_t)now, (g.pctl->pctldb)->n);

  for(i = 0; i <= g.queue_numchannels - 1; ++i){
    if(g.qtable != NULL)
      n = (g.qtable->nbspq[i])->n;
    else
      n = 0;

    fprintf(f, " %" PRIu32, n);
  }

  fprintf(f, "\n");

  fclose(f);
}

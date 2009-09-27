/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "../readn.h"
#include "../slave.h"
#include "../err.h"
#include "../signal.h"
#include "../globals.h"
#include "../util.h"
#include "../packfp.h"	/* init, cleanup packetinfo */
#include "../pctl.h"
#include "../file.h"
#include "slavein.h"
#include "framep.h"

/*
 * private thread info (stored in the info field of the slave_element_st)
 */
struct slavein_info_st {
  struct packet_info_st *packetinfo;
  struct pctl_element_st *pce;
  char *buffer;
  int buffer_size;
};

/*
 * NOTE: The file information read from the fifo is stored in the
 * appropriate elements of the gpce. The function int nbsfp_pack_fpath()
 * is then used to create the gpacketinfo thst is passed to the server
 * and filter queues. Only the elements of the gpce that are neeed by the
 * function nbsfp_pack_fpath() are used, namely:
 * pdh_product_seq_number, psh_product_type, psh_product_category,
 * psh_product_code, fname, fpath.
 */

static int slavein_info_create(struct slavein_info_st **slavein_info);
static void slavein_info_destroy(struct slavein_info_st *slavein_info);
static int recv_in_packet(struct slave_element_st *slave);

static int slavein_info_create(struct slavein_info_st **slavein_info){

  int pce_size;
  struct pctl_element_st *pce;
  struct packet_info_st *packetinfo;
  struct slavein_info_st *p;
  
  p = malloc(sizeof(struct slavein_info_st));
  if(p == NULL){
    log_err("Cannot initalize the the slavein info.");
    return(-1);
  }

  p->packetinfo = NULL;
  p->pce = NULL;
  p->buffer = NULL;
  p->buffer_size = 0;

  /*
   * Initialize the pce, but without the memory file since it will
   * not be needed.
   */
  pce_size = const_pce_size();
  if((pce = malloc(pce_size)) == NULL){
    free(p);
    log_err("Cannot initalize slavein the receive pce.");
    return(-1);
  }
  pce->fpath_size = const_fpath_maxsize() + 1;

  if(nbsfp_packetinfo_create(&packetinfo) != 0){
    free(pce);
    free(p);
    log_err("Cannot initalize the slavein packetinfo.");
    return(-1);
  }
  
  p->packetinfo = packetinfo;
  p->pce = pce;
  *slavein_info = p;  

  return(0);
}

static void slavein_info_destroy(struct slavein_info_st *slavein_info){

  if(slavein_info->packetinfo != NULL)
    nbsfp_packetinfo_destroy(slavein_info->packetinfo);

  if(slavein_info->pce != NULL)
    free(slavein_info->pce);

  if(slavein_info->buffer != NULL)
    free(slavein_info->buffer);

  free(slavein_info);
}

int slavein_init(struct slave_element_st *slave){

  struct slavein_info_st *p;
  int status;

  status = slavein_info_create(&p);
  if(status == 0)
    slave->info = (void*)p;

  return(status);
}

void slavein_cleanup(struct slave_element_st *slave){

  if(slave->info == NULL)
    return;

  slavein_info_destroy((struct slavein_info_st*)slave->info);
  slave->info = NULL;
}

int slavein_loop(struct slave_element_st *slave){
  /*
   * After a reading error, it is best to close and reopen the fifo.
   * This function returns 1 when there is such an error, or 2 when
   * there is a processig error, or 0 when there are no errors.
   */
  int status = 0;
  int cancel_state;

  pthread_testcancel();

  status = recv_in_packet(slave);

  if(status == -1)
    log_err2("Error reading from", slave->infifo);
  else if(status == -2)
    log_info("Timed out watiting to read from %s", slave->infifo);
  else if(status == 1)
    log_info("Bad input from %s", slave->infifo);
  else if(status >= 2)
    log_errx("Corrupt packet error [%d] reading from %s",
	     status, slave->infifo);

  /*
   * We close the input fifo on an error. If there has not been any
   * input, we don't close since that is not a real error.
   */ 
  if(status != 0){
    if(status == -2)
      return(0);
    else{
      slave_stats_update_connect_errors(slave);
      return(1);
    }
  }

  (void)pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cancel_state);
  status = slaveinproc(((struct slavein_info_st*)slave->info)->packetinfo);
  (void)pthread_setcancelstate(cancel_state, &cancel_state);

  if(status != 0){
    slave_stats_update_errors(slave);
    return(2);
  }

  slave_stats_update_packets(slave,
	(((struct slavein_info_st*)slave->info)->packetinfo)->packet_size);
  slave_stats_report(slave);
  
  return(status);
}

static int recv_in_packet(struct slave_element_st *slave){
  /*
   * The format of the packet received from the fifo is a file info
   * string with the format
   *
   * "%u %d %d %d %d %s %s\n" seq type cat code npchindex fname fpath
   *
   * and this preceeded by four bytes (in big endian order) indicating
   * the size of that string, not including the final '\n'.
   */
  int status = 0;
  int finfo_size;
  unsigned char finfo_size_buf[4];
  int n;
  char *fpath;
  char *fname;
  char *fbasename;
  int fpath_len;
  int fname_len;
  int fbasename_len;
  struct slavein_info_st *slavein_info = (struct slavein_info_st*)slave->info;

  n = readn(slave->slave_fd, finfo_size_buf, 4,
	    (unsigned int)slave->options.slave_read_timeout_secs, 0);
  if(n == -1)
    status = -1;	/* real reading error */
  else if(n == -2)
    status = -2;	/* timed out before anything could be read */
  else if(n != 4)
    status = 1;

  if(status != 0)
    return(status);

  finfo_size = unpack_uint32(finfo_size_buf, 0);
  if(slavein_info->buffer_size < finfo_size + 1){
    if(slavein_info->buffer != NULL)
      free(slavein_info->buffer);
    
    slavein_info->buffer_size = finfo_size + 1;
    slavein_info->buffer = malloc(slavein_info->buffer_size);
  }

  if(slavein_info->buffer == NULL){
    slavein_info->buffer_size = 0;
    return(-1);
  }

  n = readn(slave->slave_fd, slavein_info->buffer, finfo_size + 1,
	    (unsigned int)slave->options.slave_read_timeout_secs, 0);
  if(n == -1)
    status = -1;
  else if(n != finfo_size + 1)
    status = 1;

  if(status != 0)
    return(status);

  if(slavein_info->buffer[finfo_size] != '\n')
    return(2);

  slavein_info->buffer[finfo_size] = '\0';

  if(sscanf(slavein_info->buffer, "%u %d %d %d %d",
	    &(slavein_info->pce)->seq_number,
	    &(slavein_info->pce)->psh_product_type,
	    &(slavein_info->pce)->psh_product_category,
	    &(slavein_info->pce)->psh_product_code,
	    &(slavein_info->pce)->np_channel_index) != 5){
    return(3);
  }

  /*
   * fpath starts after the last blank.
   */
  fpath = strrchr(slavein_info->buffer, ' ');
  if(fpath == NULL)
    return(4);

  *fpath = '\0';	/* now fname starts after the last blank as well */
  fname = strrchr(slavein_info->buffer, ' ');
  if(fname == NULL)
    return(5);

  ++fpath;
  ++fname;

  fpath_len = strlen(fpath);
  if(fpath_len + 1 > (slavein_info->pce)->fpath_size)
    return(6);

  strncpy((slavein_info->pce)->fpath, fpath, fpath_len + 1);

  /*
   * Find the start of the basename.
   */
  fbasename = findbasename((slavein_info->pce)->fpath);
  if(fbasename == NULL)
    return(7);

  fbasename_len = strlen(fbasename);
  if(fbasename_len > FBASENAME_SIZE)
    return(8);

  strncpy((slavein_info->pce)->fbasename, fbasename, FBASENAME_SIZE + 1);

  fname_len = strlen(fname);
  if(fname_len > FNAME_SIZE)
    return(9);

  strncpy((slavein_info->pce)->fname, fname, FNAME_SIZE + 1);

  if(status == 0)
    status = nbsfp_pack_fpath(slavein_info->packetinfo, slavein_info->pce);

  return(status);
}

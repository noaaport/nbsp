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
#include "../packfp.h"	/* init, destroy packetinfo */
#include "../pctl.h"
#include "../file.h"
#include "slavein.h"
#include "framep.h"

/*
 * static variables
 */
static struct packet_info_st gpacketinfo = {0, 0, 0, 0, 0,
					    NULL, NULL, NULL, 0};
static struct pctl_element_st *gpce = NULL;
static char *gbuffer = NULL;
static int gbuffer_size = 0;

/*
 * NOTE: The file information read from the fifo is stored in the
 * appropriate elements of the gpce. The function int nbsfp_pack_fpath()
 * is then used to create the gpacketinfo thst is passed to the server
 * and filter queues. Only the elements of the gpce that are neeed by the
 * function nbsfp_pack_fpath() are used, namely:
 * pdh_product_seq_number, psh_product_type, psh_product_category,
 * psh_product_code, fname, fpath.
 */

static int init_slavein(void);
static void *slavein_main(void *arg);
static int slavein_loop(void);
static int recv_in_packet(void);

int spawn_slavein(void){

  int status = 0;

  status = init_slavein();

  if(status == 0)
    status = spawn_slave_thread(slavein_main);

  return(status);
}

static void *slavein_main(void *arg __attribute__((unused))){

  int status = 0;

  while(get_quit_flag() == 0){
    status = slavein_loop();
  }

  nbsfp_packetinfo_destroy_pool(&gpacketinfo);
  if(gpce != NULL){
    free(gpce);
    gpce = NULL;
  }

  if(gbuffer != NULL){
    free(gbuffer);
    gbuffer = NULL;
  }

  return(NULL);
}

static int slavein_loop(void){
  /*
   * After a reading error, it is best to close and reopen the fifo.
   */
  int status = 0;
  int cancel_state;

  pthread_testcancel();

  if(g.slave_fd == -1){
    log_errx("Reopening %s.", g.infifo);
    status = slave_reopen();
  }

  if(status != 0)
    return(status);

  status = recv_in_packet();

  if(status == -1)
    log_err2("Error reading from", g.infifo);
  else if(status == -2)
    log_info("No input from %s", g.infifo);
  else if(status == 1)
    log_errx("Timed out while reading from %s", g.infifo);
  else if(status >= 2)
    log_errx("Corrupt packet error [%d] reading from %s", status, g.infifo);

  /*
   * We close the input fifo on an error. If there has not been any
   * input, we don't close since that is not a real error; the inn feed script
   * may also be waiting for data.
   */ 
  if((status != 0) && (status != -2))
    (void)slave_close();

  if(status == 0){
    (void)pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cancel_state);
    status = slaveinproc(&gpacketinfo);
    (void)pthread_setcancelstate(cancel_state, &cancel_state);
  }

  return(status);
}

static int init_slavein(void){

  int pce_size;

  /*
   * Initialize the pce, but without the memory file since it will
   * not be needed.
   */
  pce_size = const_pce_size();
  if((gpce = malloc(pce_size)) == NULL){
    log_err("Cannot initalize the receive pce.");
    return(-1);
  }
  gpce->fpath_size = const_fpath_maxsize() + 1;

  if(nbsfp_packetinfo_init_pool(&gpacketinfo) != 0){
    free(gpce);
    gpce = NULL;
    log_err("Cannot initalize the receive packetinfo memory pool.");
    return(-1);
  }
  
  return(0);
}

static int recv_in_packet(void){
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

  n = readn(g.slave_fd, finfo_size_buf, 4,
	    (unsigned int)g.broadcast_read_timeout_s, 0);
  if(n == -1)
    status = -1;
  else if(status == -2)
    status = -2;	/* timed out before anything could be read */
  else if(n != 4)
    status = 1;

  if(status != 0)
    return(status);

  finfo_size = unpack_uint32(finfo_size_buf, 0);
  if(gbuffer_size < finfo_size + 1){
    if(gbuffer != NULL)
      free(gbuffer);
    
    gbuffer_size = finfo_size + 1;
    gbuffer = malloc(gbuffer_size);
  }

  if(gbuffer == NULL){
    gbuffer_size = 0;
    return(-1);
  }

  n = readn(g.slave_fd, gbuffer, finfo_size + 1,
	    (unsigned int)g.slave_read_timeout_s, 0);
  if(n == -1)
    status = -1;
  else if(n != finfo_size + 1)
    status = 1;

  if(status != 0)
    return(status);

  if(gbuffer[finfo_size] != '\n')
    return(2);

  gbuffer[finfo_size] = '\0';

  if(sscanf(gbuffer, "%u %d %d %d %d",
	    &gpce->seq_number, &gpce->psh_product_type,
	    &gpce->psh_product_category, &gpce->psh_product_code,
	    &gpce->np_channel_index) != 5){
    return(3);
  }

  /*
   * fpath starts after the last blank.
   */
  fpath = strrchr(gbuffer, ' ');
  if(fpath == NULL)
    return(4);

  *fpath = '\0';	/* now fname starts after the last blank as well */
  fname = strrchr(gbuffer, ' ');
  if(fname == NULL)
    return(5);

  ++fpath;
  ++fname;

  fpath_len = strlen(fpath);
  if(fpath_len + 1 > gpce->fpath_size)
    return(6);

  strncpy(gpce->fpath, fpath, fpath_len + 1);

  /*
   * Find the start of the basename.
   */
  fbasename = findbasename(gpce->fpath);
  if(fbasename == NULL)
    return(7);

  fbasename_len = strlen(fbasename);
  if(fbasename_len > FBASENAME_SIZE)
    return(8);

  strncpy(gpce->fbasename, fbasename, FBASENAME_SIZE + 1);

  fname_len = strlen(fname);
  if(fname_len > FNAME_SIZE)
    return(9);

  strncpy(gpce->fname, fname, FNAME_SIZE + 1);

  if(status == 0)
    status = nbsfp_pack_fpath(&gpacketinfo, gpce);

  return(status);
}

/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <err.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <ctype.h>
#include "globals.h"
#include "const.h"
#include "readn.h"
#include "util.h"
#include "file.h"
#include "nbspmspoolbdb.h"
#include "spooltype.h"
#include "nbs1.h"

/*
 * Functions to support the transmission of the entire files (NBS1).
 * The functions for NBS2 (fpath) are in packfp.{h,c}. This file
 * contains the "send" fucntions (used by the "master").
 * The "receive" functions are in nbs1r.c (used by the "slave").
 */

static off_t get_num_blocks(off_t fsize);
static int get_last_block_size(off_t fsize);
static int build_nbs1_header(struct nbs1_packet_st *ep);

int init_nbs_packet_st(struct nbs1_packet_st *nbs, 
		       char *fpath, uint32_t seqnumber, 
		       int psh_product_type, int psh_product_category,
		       int psh_product_code, int np_channel_index,
		       char *fname __attribute__ ((unused))){
  /*
   * Returns:
   *
   * -1 => os error
   * -2 => not found in mspoolbdb
   */
  int status = 0;
  off_t fsize;
  char *fbasename;

  nbs->fsfd = -1;
  nbs->cfd = -1;
  nbs->mfd = -1;

  if(spooltype_fsspool()){
    status = get_file_size(fpath, &fsize);
    if(status == 0){
      nbs->fsfd = open(fpath, O_RDONLY);
      if(nbs->fsfd == -1)
	status = -1;
    }
  }else if(spooltype_mspool()){
    nbs->mfd = nbsp_mspoolbdb_open(fpath);
    if(nbs->mfd < 0)
      status = nbs->mfd;
    else
      fsize = (off_t)nbsp_mspoolbdb_datasize(nbs->mfd);
  }else if(spooltype_cspool()){
    nbs->cfd = nbsp_cspoolbdb_open(fpath);
    if(nbs->cfd < 0)
      status = nbs->cfd;
    else
      fsize = (off_t)nbsp_cspoolbdb_datasize(nbs->cfd);
  }else
    status = 1;		/* invalid value of spooltype */

  if(status != 0)
    return(status);
  
  nbs->num_blocks = get_num_blocks(fsize);
  nbs->last_block_size = get_last_block_size(fsize);
  nbs->block_number = 0;
  nbs->f_zip = 0;

  nbs->seq_number = seqnumber;
  nbs->psh_product_type = psh_product_type;
  nbs->psh_product_category = psh_product_category;
  nbs->psh_product_code = psh_product_code;
  nbs->np_channel_index = np_channel_index;
  fbasename = findbasename(fpath);
  assert(strlen(fbasename) <= FBASENAME_SIZE);
  strncpy(nbs->fbasename, fbasename, FBASENAME_SIZE + 1);

  return(status);
}

void free_nbs_packet_st(struct nbs1_packet_st *nbs){

  if(nbs->fsfd != -1){
    close(nbs->fsfd);
    nbs->fsfd = -1;
  }else if(nbs->cfd != -1){
    nbsp_cspoolbdb_close(nbs->cfd);
    nbs->cfd = -1;
  }else if(nbs->mfd != -1){
    nbsp_mspoolbdb_close(nbs->mfd);
    nbs->mfd= -1;
  }
}

int build_nbs_packet(struct nbs1_packet_st *nbs){

  int status = 0;
  char *p;
  ssize_t n;
  size_t data_size;

  ++nbs->block_number;
  nbs->block_size = NBS1_BLOCK_SIZE;
  if(nbs->block_number == nbs->num_blocks)
    nbs->block_size = nbs->last_block_size;

  /*
   * Move the pointer to the start of the data.
   */
  p = (char*)nbs->packet;
  p += NBS_ENVELOPE_SIZE + NBS1_HEADER_SIZE;

  assert((nbs->fsfd != -1) || (nbs->cfd != -1) || (nbs->mfd != -1));

  if(nbs->fsfd != -1)
    n = read(nbs->fsfd, p, nbs->block_size);
  else if(nbs->cfd != -1)
    n = nbsp_cspoolbdb_read(nbs->cfd, p, nbs->block_size);
  else if(nbs->mfd != -1)
    n = nbsp_mspoolbdb_read(nbs->mfd, p, nbs->block_size);
  else{
    /*
     * This would mean the function was called despite the fact that
     * no file was opened (in init_nbs_packet_st()) and that is really
     * a bug. Return 1, and the caller should check that.
     */
    return(1);
  }

  if((n == -1) || ((size_t)n != nbs->block_size))
    return(-1);

  data_size = NBS1_HEADER_SIZE + nbs->block_size;
  nbs->packet_size = NBS_ENVELOPE_SIZE + data_size;

  status = build_nbs1_header(nbs);
  if(status == 0)
    status = nbs_pack_envelope(nbs->packet, PACKID_FDATA, data_size);

  return(status);
}

int send_nbs_packet(int fd, struct nbs1_packet_st *nbs,
		    int timeout_ms, int retry){
  /*
   * Returns:
   * -1 => write error
   *  0 -> no errors
   *  1 => could not write all (timed out).  
   */
  int status = 0;
  ssize_t n = 0;

  n = writem(fd, nbs->packet, nbs->packet_size,
	     (unsigned int)timeout_ms, retry);
  if(n == -1)
    status = -1;
  else if((size_t)n != nbs->packet_size)
    status = 1;

  /*
   * debug: log_info("f:%d", status);
   */

  return(status);
}

static off_t get_num_blocks(off_t fsize){
  
  off_t pt;

  pt = fsize/NBS1_BLOCK_SIZE;
  if((fsize % NBS1_BLOCK_SIZE) != 0)
    ++pt;

  return(pt);
}

static int get_last_block_size(off_t fsize){

  int size;
  
  size = fsize % NBS1_BLOCK_SIZE;
  if(size == 0)
    size = NBS1_BLOCK_SIZE;

  return(size);
}

static int build_nbs1_header(struct nbs1_packet_st *nbs){ 

  unsigned char *p;

  /*
   * Move past the envelope, to the start of the header.
   */
  p = (unsigned char*)nbs->packet;
  p += NBS_ENVELOPE_SIZE;

  pack_uint32(p, nbs->seq_number, 0);
  p[4] = nbs->psh_product_type;
  p[5] = nbs->psh_product_category;
  p[6] = nbs->psh_product_code;
  p[7] = nbs->np_channel_index;
  p += 8;
  strncpy((char*)p, nbs->fbasename, FBASENAME_SIZE + 1);
  p += FBASENAME_SIZE + 1;
  p[0] = nbs->f_zip;
  pack_uint16(p, (uint16_t)nbs->num_blocks, 1);  
  pack_uint16(p, (uint16_t)nbs->block_number, 3);  
  pack_uint32(p, nbs->block_size, 5);

  return(0);
}

int nbs_file_filter(char *fname __attribute__ ((unused))){
  /*
   * This function is called by the server to decide which files
   * to distribute to the nbs clients. 
   *
   * Must return:
   *   0 => send the file
   *   1 => do not send the file
   *  -1 => error; cannot tell
   */
  int status = 0;

  /*
   * Send everything.
   */

  status = 0;

  return(status);
}

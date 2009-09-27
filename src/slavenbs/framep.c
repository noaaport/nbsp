/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "../globals.h"
#include "../oscompat.h"
#include "../err.h"
#include "../pctl.h"
#include "../stats.h"
#include "../nbspre.h"
#include "framep.h"

/*
 * This is the function called by the reading routine to process
 * the file blocks received. This function mimicks the sbnproc()
 * to process the sbn frames, and the purpose is to insert the complete
 * element in the pctl when all the blocks have been received.
 */

static int estart_activepce(struct nbs1_packet_st *nbs);
static int eupdate_activepce(struct nbs1_packet_st *nbs);
static int eend_activepce(struct nbs1_packet_st *nbs);
static int split_fbasename(struct pctl_element_st *pce);
static void cvt_to_uc(char *s);
static int msave_nbs_block(struct nbs1_packet_st *nbs);

int slavenbsproc(struct nbs1_packet_st *nbs){

  int status = 0;

  update_stats_frames_received(nbs->packet_size);

  if(nbs->block_number == 1)
    status = estart_activepce(nbs);

  if(status == 0)
    status = eupdate_activepce(nbs);

  if((status == 0) && (nbs->block_number == nbs->num_blocks))
    status = eend_activepce(nbs);

  return(status);
}

static int estart_activepce(struct nbs1_packet_st *nbs){
  /*
   * This function initializes the active pce for this channel
   * in the control list. 
   *
   * Returns:
   *	 0 => no errors
   *	-1 => memory error 
   *     1 => bad wmo header
   *	 2 => fname queue key size is not large enough (configuration error).
   *	 3 => full path name of product too long
   *     4 => file rejected by filter
   */
  int status = 0;
  struct pctl_element_st *pce;
  size_t mfile_size_estimate;

  assert(nbs->block_number == 1);
  update_stats_products_transmitted();

  pce = open_pctl_activepce(g.pctl, nbs->slavenbs_reader_index);
  if(pce == NULL){
    log_info("%u started on np channel %d with unfinished product in pctl.",
	     nbs->seq_number, nbs->slavenbs_reader_index);
    
    update_stats_products_missed();
    if(esend_pctl_activepce(g.pctl, nbs->slavenbs_reader_index) != 0){
      log_errx("Could not add %u to processor queue.", nbs->seq_number);
    }
    close_pctl_activepce(g.pctl, nbs->slavenbs_reader_index);
    pce = open_pctl_activepce(g.pctl, nbs->slavenbs_reader_index);
  }

  assert(pce != NULL);

  pce->f_complete = 0;
  pce->num_fragments = nbs->num_blocks;
  pce->recv_fragments = 0;   /* this is updated by update_control_element */
  pce->seq_number = nbs->seq_number;
  pce->orig_seq_number = 0;
  pce->sbn_datstream = 0;
  pce->psh_product_type = nbs->psh_product_type;
  pce->psh_product_category = nbs->psh_product_category;
  pce->psh_product_code = nbs->psh_product_code;
  pce->np_channel_index = nbs->np_channel_index;

  /*
   * The pce->tsqueue is the time at which the product is inserted in the
   * queue, and it is used as the index in the mf db. This field 
   * is filled just before inserting the pce
   * in the pctl queue, and after locking it for insertion.
   * (in the function pctldb_send_pce() in pctldb.c).
   */
  if((status = oscompat_clock_gettime(&pce->ts)) != 0){
    assert(status == 0);
    status = 0;
    pce->ts.tv_sec = time(NULL);
    pce->ts.tv_nsec = 0;
  }

  pce->save_format = g.saveframefmt;
  pce->saveunz_flag = 0;	/* determined below */
  pce->rtxdb_flag = 0;		/* managed in the processor, not here */

  /*
   * The pce->mf is opened by open_pctl_activepce() so we do not
   * create one explicitly; but we adjust the size
   * to save calls to malloc, or save memory if it can be shrinked.
   * We ignore any errors here if the size could not be adjusted.
   */
  mfile_size_estimate = NBS1_PACKET_MAXSIZE * pce->num_fragments;
  if(mfile_size_estimate < (size_t)g.memfile_minsize)
    mfile_size_estimate = (size_t)g.memfile_minsize;

  (void)adjust_memfile_size(pce->mf, mfile_size_estimate);

  pce->ctrlhdr[0] = '\0';
  pce->ctrlhdr_size = 0;

  strncpy(pce->fbasename, nbs->fbasename, FBASENAME_SIZE + 1);
  pce->fbasename[FBASENAME_SIZE] = '\0';

  /*
   * Extract pce->fname, pce->wmo_id, pce->wmo_station,
   * pce->wmo_time, pce->wmo_awips, pce->wmo_notawips
   * and also build fpath.
   */
  status = split_fbasename(pce);

  if(status != 0){
    log_errx("Could not add element to control list. Bad fbasename %s.",
	     pce->fbasename);
    status = 1;
    goto end;
  }

  if(np_regex_match(pce->fname) == 1){
    /*
     * Rejected by filter. If there is an error (-1), we save the file
     * as a cautionary measure.
     */
    update_stats_products_rejected();
    log_verbose(3, "Rejected: %s.", pce->fname);

    status = 4;
    goto end;
  }

  if(nbs->f_zip){
    /*
     * The default is to save the frames as they are arrive. 
     * If the file is transmitted compressed, it is saved in uncompressed
     * form if it's name is _rejected_ by the the savez_regex filter
     * (if the savez_regex has been defined).
     */
    if(savez_regex_match(pce->fname) == 1)
      pce->saveunz_flag = 1;
  }

 end:

  if(status != 0){
    close_pctl_activepce(g.pctl, nbs->slavenbs_reader_index);
    if(status != 4)
      update_stats_products_missed();
  }

  return(status);
}

static int eupdate_activepce(struct nbs1_packet_st *nbs){
  /*
   * This function updates the active pce.
   *
   * Returns:
   *  0 if there are no errors.
   *  1 if the element (seqnumber) is not in the control list.
   *  2 if the block has jumped the block number (missing blocks)
   * -1 a memory or write error msave_nbs_block
   */
  int status = 0;

  /*
   * This function updates the frame counter, checking that the received
   * frame is the next one in the sequence, and the next function 
   * saves the frame in the product mfile.
   *
   * NOTE: The last argument of this function assumes a zero-based
   * block_number (as in the noaaport frames). Since we are using
   * a 1-based block number we substract 1.
   */
  status = update_activepce_framecounter(g.pctl, nbs->slavenbs_reader_index,
					 nbs->seq_number, 
					 nbs->block_number - 1);

  if(status == 1){
    log_verbose(3, "PROGRESS: %u is not active.", nbs->seq_number);
    return(1);
  }else if(status == 2){
    struct pctl_element_st *pce;

    pce = get_pctl_activepce(g.pctl, nbs->slavenbs_reader_index,
			     nbs->seq_number);
    if(pce != NULL){
      log_errx("Missing frames for %s[%d]: %d -> %d of %d: [z=%d]",
	       pce->fname,
	       pce->np_channel_index,
	       pce->recv_fragments - 1, nbs->block_number,
	       pce->num_fragments, nbs->f_zip);
    }else    
      log_errx("Missing frames for %u.", nbs->seq_number);
  }else if(status == -1){
    log_err2u("Could not update", nbs->seq_number);
  }

  if(status == 0)
    status = msave_nbs_block(nbs);    

  if(status != 0){
    update_stats_products_missed();
    close_pctl_activepce(g.pctl, nbs->slavenbs_reader_index);
  }

  return(status);
}

static int eend_activepce(struct nbs1_packet_st *nbs){

  int status = 0;

  /*
   * Get the sequence number of the active pce and check that it is
   * what was received here.
   */  
  status = complete_activepce(g.pctl, nbs->slavenbs_reader_index,
			      nbs->seq_number);
  assert(status != -1);

  if(status == 1){
    log_verbose(3, "END: %u not active on %d.", nbs->seq_number,
		nbs->slavenbs_reader_index);
  }else if(status == 2){
    log_verbose(3, "END: Incomplete frames received for %u on %d.",
		nbs->seq_number, nbs->slavenbs_reader_index);
  }else{
    update_stats_products_completed();
    status = esend_pctl_activepce(g.pctl, nbs->slavenbs_reader_index);
    if(status != 0){
      log_errx("Could not add %u to processor queue.", nbs->seq_number);
    }
  }

  close_pctl_activepce(g.pctl, nbs->slavenbs_reader_index);

  return(status);
}

static int split_fbasename(struct pctl_element_st *pce){

  int status = 0;
  int fbasename_size;
  char fbasename[FBASENAME_SIZE + 1];
  char *p1, *p2;
  int fnamesize;
  int n;

  fbasename_size = strlen(pce->fbasename);
  if(fbasename_size > FBASENAME_SIZE)
    return(1);

  strncpy(fbasename, pce->fbasename, FBASENAME_SIZE + 1);

  p2 = strchr(fbasename, '.');
  if(p2 == NULL)
    return(1);

  *p2 = '\0';
  p1 = &fbasename[0];
  fnamesize = strlen(p1);
  if(fnamesize > FNAME_SIZE)
    return(1);
     
  strncpy(pce->fname, p1, FNAME_SIZE + 1);

  pce->wmo_time[0] = '\0';
  pce->wmo_awips[0] = '\0';
  pce->wmo_notawips[0] = '\0';

  p2 = strchr(fbasename, FNAME_WMOID_SEP_CHAR);
  if(p2 == NULL)
    return(1);

  *p2 = '\0';
  if(strlen(p1) > WMO_STATION_SIZE)
    return(1);

  strncpy(pce->wmo_station, p1, WMO_STATION_SIZE + 1);

  p1 = (p2 + 1);
  p2 = strchr(p1, FNAME_AWIPS_SEP_CHAR);
  if(p2 != NULL){
    *p2 = '\0';
    if(strlen(p1) > WMO_ID_SIZE)
      return(1);

    strncpy(pce->wmo_id, p1, WMO_ID_SIZE + 1);
    p1 = p2 + 1;

    if(strlen(p1) > WMO_AWIPS_SIZE)
      return(1);
    
    strncpy(pce->wmo_awips, p1, WMO_AWIPS_SIZE + 1);
  }else{
    p2 = strchr(p1, FNAME_NOTAWIPS_SEP_CHAR);
    if(p2 != NULL){
      *p2 = '\0';
      if(strlen(p1) > WMO_ID_SIZE)
	return(1);

      strncpy(pce->wmo_id, p1, WMO_ID_SIZE + 1);
      p1 = p2 + 1;
      
      if(strlen(p1) > WMO_NOTAWIPS_SIZE)
	return(1);
    
      strncpy(pce->wmo_notawips, p1, WMO_NOTAWIPS_SIZE + 1);
    }else{
      if(strlen(p1) > WMO_ID_SIZE)
	return(1);
      
      strncpy(pce->wmo_id, p1, WMO_ID_SIZE + 1);
    }
  }

  /*
   * Build the fpath.
   */
  n = snprintf(pce->fpath, pce->fpath_size, "%s/%s/%s", g.spooldir, 
	       pce->wmo_station, pce->fbasename); 

  if(n >= pce->fpath_size)
    return(1);

  cvt_to_uc(pce->wmo_station);
  cvt_to_uc(pce->wmo_id);
  cvt_to_uc(pce->wmo_awips);
  cvt_to_uc(pce->wmo_notawips);

  return(status);
}

static void cvt_to_uc(char *s){

  char *p = s;

  while(*p != '\0'){
    *p = toupper(*p);
    ++p;
  }
}

static int msave_nbs_block(struct nbs1_packet_st *nbs){
  /*
   * This function is called after updating the control list in
   * the second stage.
   */
  int status = 0;
  struct pctl_element_st *pce;
  struct memfile_st *mf = NULL;
  unsigned char f_compressed;
  int save_format;
  int saveunz_flag;

  pce = get_pctl_activepce(g.pctl, nbs->slavenbs_reader_index,
			   nbs->seq_number);
  if(pce == NULL)
    return(1);

  save_format = pce->save_format;	/* not currently used */
  saveunz_flag = pce->saveunz_flag;
  mf = pce->mf;
  f_compressed = nbs->f_zip;

  /*
   * This comment is adapted from the comment in msave_frame() in ../framep.c.
   * If saveunz_flag is not set, then all the frames are saved as they arrive.
   * If saveunz_flag is set, then the frames are saved as they
   * arrive; but the first byte of each saved frame is the compressed flag.
   * the compressed ones will be uncompressed when saving the file to
   * disk by mcat_frames().
   */

  if(saveunz_flag == 0){
    if(write_memfile(mf, nbs->block, nbs->block_size) < 0)
      status = -1;
  }else{
    if(write_memframe(mf, nbs->block, nbs->block_size, f_compressed) < 0)
      status = -1;
  }

  if(status == -1){
    log_err("Could not save frame.");
    log_errx("%u.%d", nbs->seq_number, nbs->block_number);
  }

  return(status);
}

/*
 * Test split_fbasename
 *
int main(int argc, char ** argv){

  int status = 0;
  struct pctl_element_st pce;
  
  strncpy(pce.fbasename, argv[1], FBASENAME_SIZE + 1);

  status = split_fbasename(&pce);
  if(status != 0)
    errx(1, "Bad fbasename.");

  fprintf(stdout, "%s\n", pce.fname);
  fprintf(stdout, "%s\n", pce.wmo_station);
  fprintf(stdout, "%s\n", pce.wmo_id);
  fprintf(stdout, "%s\n", pce.wmo_awips);
  fprintf(stdout, "%s\n", pce.wmo_notawips);
  fprintf(stdout, "%s\n", pce.wmo_time);

  return(0);
}
*/

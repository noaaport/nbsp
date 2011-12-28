/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "globals.h"
#include "oscompat.h"
#include "err.h"
#include "sbn.h"
#include "nbsp.h"
#include "qstate.h"
#include "nbspre.h"
#include "framep.h"

/*
 * This holds the information from the last frame received in each channel;
 * this is mostly to keep track of frame jumps.
 */
static struct frame_processor_st {
  struct frame_level_header fh[NPCAST_NUM_CHANNELS];
} gframep;

static void update_framep_info(struct sbn_frame *sbnf);  
static int e_sbn_command_nondata(struct sbn_frame *sbnf);
static int e_sbn_unpack_frame(struct sbn_frame *sbnf);
static void abort_activepce(struct sbn_frame *sbnf);
static int e_start_activepce(struct sbn_frame *sbnf);
static int start_activepce(struct sbn_frame *sbnf);
static int e_update_activepce(struct sbn_frame *sbnf);
static int update_activepce(struct sbn_frame *sbnf);
static int e_end_activepce(struct sbn_frame *sbnf);
static int msave_frame(struct sbn_frame *sbnf);
static int make_product_fpath(struct pctl_element_st *pce);
static void make_product_fname(char *fname, char *wmo_id, char *wmo_station,
				  char *wmo_awips, char *wmo_notawips,
				  int producttype);
static int make_product_fbasename(struct pctl_element_st *pce);
static void cvt_to_lc(char *p);
static void log_pce_diag(struct sbn_frame *sbnf);

int sbnframeproc(struct sbn_frame *sbnf){
  /*
   * The frame processor is a three-stage process. 
   * We check the sbnf->pdh.transfer_type flags. If the 
   * START flag is set, we add the product to the control list.
   * If the PROGRESS flag is set, we update the corresponding element
   * in the list. If the END flag is set, the product element is
   * flaged as processable (now processed by a separate processing thread).
   */
  int transfertype;
  int status = 0;
  
  status = e_sbn_unpack_frame(sbnf);
  if(status != 0)
    return(status);

  if(sbnf->fh.sbn_command != SBN_COMMAND_DATA_TRANSFER){
    status = e_sbn_command_nondata(sbnf);
    
    return(status);
  }

  transfertype = sbnf->pdh.transfer_type;
  if(transfertype & (PDH_TRANSFERTYPE_ERROR | PDH_TRANSFERTYPE_ABORT)){
    abort_activepce(sbnf);
    log_info("Aborted %u", sbnf->pdh.product_seq_number);
    return(0);
  }

  if(transfertype & PDH_TRANSFERTYPE_START){
    if(sbnf->psh.transfer_status_flag & PSH_STATUSFLAG_RETRANSMIT){
      update_stats_products_retransmitted();
      log_verbose(1, "%u is retransmitting %u.",
		  sbnf->pdh.product_seq_number, sbnf->psh.orig_seq_number);
    }else{
      /*
       * The documentation says that the original sequence number field
       * is non-zero and used only in retransmissions, and it is zero
       * otherwise. Just to make sure we set it here to zero.
       * (This is used later to decide whether or not to process the product.)
       */
      sbnf->psh.orig_seq_number = 0;
    }

    status = e_start_activepce(sbnf);
  }

  if(status == 0){
    if(transfertype & PDH_TRANSFERTYPE_PROGRESS)
      status = e_update_activepce(sbnf);
  }

  if(status == 0){
    if(transfertype & PDH_TRANSFERTYPE_END){
      /*
       * Just flag the element as complete and ready for the processing
       * thread.
       */
      status = e_end_activepce(sbnf);
    }
  }

  return(status);
}

static int e_sbn_unpack_frame(struct sbn_frame *sbnf){

  int status = 0;

  update_stats_frames_received(sbnf->rawdata_size);

  status = sbn_unpack_frame(sbnf);
  if(status == 1)
    log_errx("Check sum error.");
  else if(status == 2)
    log_errx("Inconsistent frame.");
  else if(status == 3){
    /*
     * Don't count this as a bad frame (in the statistics).
     */
    log_errx("Uncompress error %d unpacking frame.", sbnf->f_unzstatus);
  }

  if((status == 0) || (status == 3))
    update_stats_frames(0);
  else{
    update_stats_frames(status);
    return(status);
  }

  update_framep_info(sbnf);  
  
  if(status == 0){
    log_verbose(3, "Unpacked sbn frame: %u.", sbnf->fh.sbn_seq_number);
  }

  return(status);
}

static int e_sbn_command_nondata(struct sbn_frame *sbnf){
  /*
   * sbn_command 5 is Synchronization timing packet, and 3 is test.
   */
  int status = 0;

  if(sbnf->fh.sbn_command == SBN_COMMAND_SYNC_TIMING)
    log_verbose(1, "Sync timing command on channel %d.",
		sbnf->np_channel_index);
  else if(sbnf->fh.sbn_command == SBN_COMMAND_TEST_MSG)
    log_verbose(1, "Test message command on channel %d.",
		sbnf->np_channel_index);
  else
    log_verbose(1, "sbn command %d command on channel %d.",
		sbnf->fh.sbn_command, sbnf->np_channel_index);

  return(status);
}

static void update_framep_info(struct sbn_frame *sbnf){
  /*
   * Check the sbn sequence number to see if there was a gap
   * skipping the first time (program startup).
   */
  unsigned int last, current;

  last = gframep.fh[sbnf->np_channel_index].sbn_seq_number;
  current = sbnf->fh.sbn_seq_number;

  if((last > 0) && (current > last + 1)){
    if(sbnf->fh.sbn_command == SBN_COMMAND_DATA_TRANSFER)
      log_errx("While processing %d.%u.%d: SBN seqnum jumped from %u to %u",
	       sbnf->np_channel_index,
	       sbnf->pdh.product_seq_number, sbnf->pdh.block_number,
	       last, current);
    else
      log_errx("While processing %d: SBN seqnum jumped from %u to %u",
	       sbnf->np_channel_index,
	       last, current);

    update_stats_frames_jumps();
  }	       	       

  memcpy(&gframep.fh[sbnf->np_channel_index], &sbnf->fh,
	 sizeof(struct frame_level_header));
}

static void abort_activepce(struct sbn_frame *sbnf){  
  /*
   * If the abort or error flag is set in the transfer type of the frame,
   * the active pce for this channel is killed.
   */

  update_stats_products_aborted();
  log_verbose(1, "ABORT: received for %u on channel %d.",
	      sbnf->pdh.product_seq_number, sbnf->np_channel_index);

  close_pctl_activepce(g.pctl, sbnf->np_channel_index);
}

static int e_start_activepce(struct sbn_frame *sbnf){

  int status = 0;

  if(sbnf->pdh.block_number != 0){
    log_verbose(1, "Transfer type has START but blocknumber is not 0");
    return(1);
  }

  update_stats_products_transmitted();

  status = start_activepce(sbnf);

  if(status == -1)
    log_err("Could not add element to control list.");
  else if(status == 1){
    log_errx("Could not add element to control list. Bad product header.");
    log_debug("bad header: %.80s", sbnf->pdata);
  }else if(status == 2)
    log_errx("Could not add element to control list: fname qkey too long.");
  else if(status == 3)
    log_errx("Could not add element to control list. Full path too long.");
  else if(status == 4){
    /*
     * The file was rejected by the filter.
     */
    update_stats_products_rejected();
  }else{
    log_verbose(2, "START: ");
    log_pce_diag(sbnf);
  }
  
  if((status != 0) && (status != 4))
    update_stats_products_missed();

  return(status);
}

static int start_activepce(struct sbn_frame *sbnf){
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
  struct pctl_element_st *pce;
  int status = 0;
  size_t mfile_size_estimate;

  assert(sbnf->pdh.block_number == 0);

  pce = open_pctl_activepce(g.pctl, sbnf->np_channel_index);
  if(pce == NULL){
    log_info("%u started on np channel %d with unfinished product in pctl.",
	     sbnf->pdh.product_seq_number, sbnf->np_channel_index);

    update_stats_products_missed();
    if(esend_pctl_activepce(g.pctl, sbnf->np_channel_index) != 0){
      log_errx("Could not add element to processor queue.");
    }
    close_pctl_activepce(g.pctl, sbnf->np_channel_index);
    pce = open_pctl_activepce(g.pctl, sbnf->np_channel_index);
  }

  assert(pce != NULL);

  pce->f_complete = 0;
  pce->num_fragments = sbnf->psh.num_fragments;
  /*
   * When there is only one frame, the SBN protocol sets this number
   * (in the PSH) to 0
   */
  if(sbnf->psh.num_fragments == 0)
    pce->num_fragments = 1;

  pce->recv_fragments = 0;   /* this is updated by update_control_element */
  pce->seq_number = sbnf->pdh.product_seq_number;
  pce->orig_seq_number = sbnf->psh.orig_seq_number;
  pce->sbn_datstream = sbnf->fh.sbn_datstream;
  pce->psh_product_type = sbnf->psh.product_type;
  pce->psh_product_category = sbnf->psh.product_category;
  pce->psh_product_code = sbnf->psh.product_code;
  pce->np_channel_index = sbnf->np_channel_index;

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

  pce->save_format = g.spoolsavefmt;
  pce->saveunz_flag = 0;	/* determined below */
  pce->rtxdb_flag = 0;		/* managed in the processor, not here */

  /*
   * The pce->mf is opened by open_pctl_activepce() so we do not
   * create one explicitly; but we adjust the size
   * to save calls to malloc, or save memory if it can be shrinked.
   * We ignore any errors here if the size could not be adjusted.
   */
  mfile_size_estimate = g.memfile_blocksize * pce->num_fragments;
  if(mfile_size_estimate < (size_t)g.memfile_minsize)
    mfile_size_estimate = (size_t)g.memfile_minsize;

  (void)adjust_memfile_size(pce->mf, mfile_size_estimate);

  pce->ctrlhdr_size = MAX_CTRLHDR_SIZE;
  status = copy_ctrlheader(pce->ctrlhdr, &pce->ctrlhdr_size, sbnf);
  if(status == 1){
    /* 
     * There is no ctrlheader in the transmission. This is not an error
     * since the ctrlheader is not always transmitted.
     */
    pce->ctrlhdr[0] = '\0';
    pce->ctrlhdr_size = 0;
    status = 0;
  }

  pce->f_has_ccb = 0;
  if(sbnf->ccb != NULL)
    pce->f_has_ccb = 1;

  status = split_wmo_header(sbnf, 
			    pce->wmo_id, pce->wmo_station, 
			    pce->wmo_time, pce->wmo_awips, pce->wmo_notawips);
  if(status != 0){
    status = 1;
    goto end;
  }

  make_product_fname(pce->fname, 
		     pce->wmo_id, pce->wmo_station, pce->wmo_awips,
		     pce->wmo_notawips, pce->psh_product_type);

  if(make_product_fbasename(pce) != 0){
    status = 2;
    goto end;
  }

  if(make_product_fpath(pce) != 0){
    status = 3;
    goto end;
  }

  if(np_regex_match(pce->fname) == 1){
    /*
     * Rejected by filter. If there is an error (-1), we save the file
     * as a cautionary measure.
     */
    log_verbose(3, "Rejected: %s.", pce->fname);

    status = 4;
    goto end;
  }

  if(pce->orig_seq_number != 0){
   /*
    * The indication of a retransmission is a non-zero value of this
    * variable. If it is a retransmission, check the high load flag
    * and the high load retransmissions index, and proceed accordingly.
    * (See load.c)
    */
    if(g.f_loadave_max_rtx == 1){
      log_warnx("Rejecting %s due to high load retransmission index.",
		pce->fname);
      status = 4;
      goto end;
    }
  }

  if(sbnf->f_frdata_compressed){
    /*
     * If the file is transmitted compressed, the default is to saved it
     * compressed. It is saved in uncompressed form if it's name 
     * is _rejected_ by the savez_regex_match filter
     *  and if it was transmitted with a ctrlheader.
     */
    if((savez_regex_match(pce->fname) == 1) && (pce->ctrlhdr_size != 0))
      pce->saveunz_flag = 1;
  }

 end:

  if(status != 0)
    close_pctl_activepce(g.pctl, sbnf->np_channel_index);    

  return(status);
}

static int e_update_activepce(struct sbn_frame *sbnf){
  
  int status = 0;

  status = update_activepce(sbnf);

  if(status == 1){
    log_verbose(3, "PROGRESS: %u is not active.",
		sbnf->pdh.product_seq_number);
    status = 0;
  }else if(status == 2){
    struct pctl_element_st *pce;

    pce = get_pctl_activepce(g.pctl, sbnf->np_channel_index,
			     sbnf->pdh.product_seq_number);
    if(pce != NULL){
      log_errx("Missing frames for %s[%d]: %d -> %d of %d: [z=%d]",
	       pce->fname,
	       pce->np_channel_index,
	       pce->recv_fragments - 1, sbnf->pdh.block_number,
	       pce->num_fragments, sbnf->f_frdata_compressed);
    }else    
      log_errx("Missing frames for %u.", sbnf->pdh.product_seq_number);
  }else if(status == -1){
    log_err2u("Could not update", sbnf->pdh.product_seq_number);
  }else{
    log_verbose(3, "PROGRESS: ");
    log_pce_diag(sbnf);
  }

  if(status != 0){
    update_stats_products_missed();
    if(esend_pctl_activepce(g.pctl, sbnf->np_channel_index) != 0){
      log_errx("Could not add element to processor queue.");
    }
    close_pctl_activepce(g.pctl, sbnf->np_channel_index);
  }

  return(status);
}

static int update_activepce(struct sbn_frame *sbnf){
  /*
   * This function updates the active pce.
   *
   * Returns:
   *  0 if there are no errors.
   *  1 if the element (seqnumber) is not in the control list.
   *  2 if the frame has jumped the block number (missing frames)
   * -1 a memory or write error msave_frame
   */
  int status = 0;

  /*
   * This function updates the frame counter, checking that the received
   * frame is the next one in the sequence, and the next function 
   * saves the frame in the product mfile.
   */
  status = update_activepce_framecounter(g.pctl,
					 sbnf->np_channel_index,
					 sbnf->pdh.product_seq_number, 
					 sbnf->pdh.block_number);

  if(status == 0)
    status = msave_frame(sbnf);    

  return(status);
}

static int e_end_activepce(struct sbn_frame *sbnf){

  int status = 0;

  /*
   * Get the sequence number of the active pce and check that it is
   * what was received here.
   */  
  status = complete_activepce(g.pctl, sbnf->np_channel_index,
			      sbnf->pdh.product_seq_number);
  assert(status != -1);

  if(status == 1){
    log_verbose(3, "END: %u not active on %d.", sbnf->pdh.product_seq_number,
		sbnf->np_channel_index);
  }else{
    if(status == 0)
	update_stats_products_completed();
    else if(status == 2){
      update_stats_products_missed();
      log_verbose(3, "END: Incomplete frames received for %u on %d.",
		sbnf->pdh.product_seq_number, sbnf->np_channel_index);
    }

    status = esend_pctl_activepce(g.pctl, sbnf->np_channel_index);
    if(status != 0)
      log_errx("Could not add %u to processor queue.",
	       sbnf->pdh.product_seq_number);
  }

  close_pctl_activepce(g.pctl, sbnf->np_channel_index);

  return(status);
}

static void make_product_fname(char *fname, char *wmo_id, char *wmo_station,
			       char *wmo_awips, char *wmo_notawips,
			       int producttype __attribute__ ((unused))){
  /*
   * The file name of the product is the station name, followed
   * by the wmo_id, then the awips if it exists, or whatever appears in the 
   * second line, provided the second line starts with an isalpha() char.
   * The separating character between the second and third pieces
   * different depending on whether there is an awips or not. 
   * This is the name we store in the pce->fname.
   *
   * NOTE: The station should be 4 characters and the wmoid whould be 6, 
   * but at least in one case that does not happen:
   * (e.g., knka_ubus1+vtp.191800_3298904, the wmoid is 5 chars).
   */
  int n = 0;

  strncpy(fname, wmo_station, WMO_STATION_SIZE);
  n += strlen(wmo_station);		/* see note above */
  fname[n++] = FNAME_WMOID_SEP_CHAR;
  fname[n] = '\0';
  strncat(fname, wmo_id, WMO_ID_SIZE);
  n += strlen(wmo_id);

  if(wmo_awips[0] != '\0'){
    fname[n++] = FNAME_AWIPS_SEP_CHAR;
    fname[n] = '\0';
    strncat(fname, wmo_awips, WMO_AWIPS_SIZE);
    n += strlen(wmo_awips);
  }else if(wmo_notawips[0] != '\0'){
    fname[n++] = FNAME_NOTAWIPS_SEP_CHAR;
    fname[n] = '\0';
    strncat(fname, wmo_notawips, WMO_NOTAWIPS_SIZE);
    n += strlen(wmo_notawips);
  }

  assert(n <= FNAME_SIZE);

  /*
   * All lower case
   */
  cvt_to_lc(fname);
}

static int make_product_fbasename(struct pctl_element_st *pce){
  /*
   * This function builds the base name of the file, pce->fbasename,
   * which consists of the pce->fname followed by the sequence key,
   * both pieces separated by a period. The sequence key is the ddhhmm
   * part of the wmo header followed by the sequence number, both
   * pieces separated by a '_'.
   */
  int n;
  int fbasenamesize = FBASENAME_SIZE + 1;
  unsigned int seqnum;

  seqnum = (pce->seq_number & 0xffffffff);	/* ensure 32 bits */
  n = snprintf(pce->fbasename, fbasenamesize, "%s%c%s%c%u",
	       pce->fname, FNAME_FSEQKEY_SEPCHAR,
	       pce->wmo_time, FNAME_SEQNUM_SEPCHAR, seqnum); 
  assert(n < fbasenamesize);

  return((n >= fbasenamesize) ? 1 : 0);
}

static int make_product_fpath(struct pctl_element_st *pce){
  /*
   * This function builds the _full path_ of the product file. The
   * pce->fname contains the root name of the file as is stored there
   * when the first frame is received, and pce->fbasename is the
   * basename we will use, which is the fname.ext, where the ext
   * is the character string built from the seq number (see above function).
   */
  int n;
  int fpath_size;
  char station[WMO_STATION_SIZE + 1]; /* lower case station */

  strncpy(station, pce->wmo_station, WMO_STATION_SIZE + 1);
  cvt_to_lc(station);

  fpath_size = pce->fpath_size;
  n = snprintf(pce->fpath, fpath_size, "%s/%s/%s", g.spooldir, 
	       station, pce->fbasename); 

  return((n >= fpath_size) ? 1 : 0);
}

int const_fpath_maxsize(void){
  /*
   * Returns (computes) the maximum value of an fpath (without the '\0')
   * given the convention above for determining the fpath. This value is needed
   * in particular by the function that initalizes the queue db,
   * which uses fixed length (db Queue type) records.
   */
  int maxsize;

  maxsize = strlen(g.spooldir) + WMO_STATION_SIZE + FBASENAME_SIZE + 2;

  return(maxsize);
}

static int msave_frame(struct sbn_frame *sbnf){
  /*
   * This function is called after updating the control list in
   * the second stage.
   */
  int status = 0;
  struct pctl_element_st *pce;
  struct memfile_st *mf = NULL;
  unsigned char f_compressed;
  int saveunz_flag;

  pce = get_pctl_activepce(g.pctl, sbnf->np_channel_index,
			   sbnf->pdh.product_seq_number);

  if(pce == NULL)
    return(1);

  saveunz_flag = pce->saveunz_flag;
  mf = pce->mf;
  f_compressed = sbnf->f_frdata_compressed;

  /*
   * If saveunz_flag is not set, then all the frames are saved as they arrive,
   * and the compressed products will have the full ctrlhdr before the
   * (compressed) first frame.
   *
   * If saveunz_flag is set, then the frames are saved as they
   * arrive; but each frame is preceeded by the header that contains
   * the the compressed flag and frame size.
   * the compressed ones will be uncompressed when saving the file to
   * disk by pce_spool_product().
   *
   * Notice that if this is a product that is transmitted with a CCB
   * then, in any case below, after assembling the frames the final
   * product memory file contains the CCB.
   */

  if(saveunz_flag == 0){
    if((sbnf->pdh.block_number == 0) && (sbnf->ctrlhdr != NULL)){
      if(write_memfile(mf, sbnf->ctrlhdr, sbnf->ctrlhdr_size) < 0)
	status = -1;
    }

    if(status == 0){
      if(write_memfile(mf, sbnf->frdata, sbnf->frdata_size) < 0)
	status = -1;
    }
  }else{
    if(write_memframe(mf, sbnf->frdata, sbnf->frdata_size, f_compressed) < 0)
      status = -1;
  }

  if(status == -1){
    log_err("Could not save frame.");
    log_errx("%u.%d", sbnf->pdh.product_seq_number, sbnf->pdh.block_number);
  }

  return(status);
}

static void cvt_to_lc(char *s){

  char *p = s;

  while(*p != '\0'){
    *p = tolower(*p);
    ++p;
  }
}

static void log_pce_diag(struct sbn_frame *sbnf){

  struct pctl_element_st *pce;

  pce = get_pctl_activepce(g.pctl, sbnf->np_channel_index,
			   sbnf->pdh.product_seq_number);
  
  log_verbose(2, " %s:%d:%d:%d", pce->fname, pce->seq_number,
	      pce->num_fragments, pce->recv_fragments);

}

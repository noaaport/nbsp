/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <inttypes.h>
#include "util.h"
#include "err.h"
#include "unz.h"
#include "signal.h"
#include "sbn.h"
#include "pctl.h"
#include "file.h"
#include "efile.h"
#include "mfile.h"
#include "stats.h"
#include "qstate.h"
#include "nbspre.h"
#include "filters.h"
#include "packfp.h"
#include "nbspdb.h"
#include "nbspq.h"
#include "spooldb.h"
#include "nbsp.h"
#include "globals.h"

static int process_pctl(void);
static int process_pctl_element(struct pctl_element_st *pce);
static int mcat_frames(struct pctl_element_st *pce);
static struct memfile_st *reopen_nbspproc_memfile(void);
static void nbsp_periodic(void);
static int init_packetinfo_packet_pool(void);
static void destroy_packetinfo_packet_pool(void);
static int e_export_product(struct pctl_element_st *pce);
static void nbspdb_flag_filter(struct pctl_element_st *pce);
static void log_missing(struct pctl_element_st *pce);
static void log_rtx(struct pctl_element_st *pce);
static void write_log_pce(FILE *f, struct pctl_element_st *pce);
/* the procsssing thread functions */
static int create_processor_thread(void);
static void *processor_thread_main(void *data);
/*
 * The higher level spooldb functions.
 */
static int nbsp_spooldb_open(void);
static void nbsp_spooldb_close(void);
static int nbsp_spooldb_insert(char *fpath);
static int nbsp_spooldb_delete_fpath(char *fpath);

/* processor's working packetinfo for sending to filter and server queues */
static struct packet_info_st gpacketinfo = {0, 0, 0, 0, 0,
					    NULL, NULL, NULL, 0};

static int init_packetinfo_packet_pool(void){

  if(nbsfp_packetinfo_init_pool(&gpacketinfo) != 0){
    log_err("Cannot initalize the processor packetinfo memory pool.");
    return(-1);
  }

  return(0);
}

static void destroy_packetinfo_packet_pool(void){

  nbsfp_packetinfo_destroy_pool(&gpacketinfo);
}

int spawn_nbsproc(void){

  int status = 0;

  status = init_nbsproc();

  if(status == 0)
    status = create_processor_thread();

  return(status);
}

void kill_processor_thread(void){
  /*
   * This "joins" the product processor thread. 
   */
  
  if(g.f_processor_thread_created == 1){
    pthread_join(g.processor_thread_id, NULL);
    log_info("Finished processor.");
  }
}

static int create_processor_thread(void){

  int status = 0;
  pthread_t t_id;
  pthread_attr_t attr;

  status = pthread_attr_init(&attr);

  if(status == 0)
    status = pthread_create(&t_id, &attr, processor_thread_main, NULL);

  if(status != 0){
    log_err("Cannot create processor thread.");
  }else{
    g.processor_thread_id = t_id;
    g.f_processor_thread_created = 1;

    log_info("Spawned processor thread.");
  }

  return(status);
}

static void *processor_thread_main(void *data){

  int status;

  while(get_quit_flag() == 0){
    status = nbsproc();
  }

  return(NULL);
}

int init_nbsproc(void){

  int status = 0;

  /* The spool directory db */
  status = nbsp_spooldb_open();
  
  if(status == 0)
    status = init_packetinfo_packet_pool();

  return(status);
}

void close_nbsproc(void){

  destroy_packetinfo_packet_pool();
  nbsp_spooldb_close();
}

int nbsproc(void){
  /*
   * This is the main function of the processing thread. It waits for
   * the pctl to advertise that it has at least one element complete
   * and ready for processing, processes the first element that it finds
   * ready, and then does some maintenance operations on the pctl.
   */
  int status;

  status = process_pctl();

  if(status == 0)
    nbsp_periodic();

  return(status);
}

int init_pctl(void){
  /*
   * This function initalizes the product control list, used by
   * the processor thread and readers.
   */
  int status = 0;
  struct pctl_st *pctl;
  struct pctldb_param_st param;
  int num_active_channels;

  param.dbenv =  g.dbenv;
  param.dbname = NULL;
  if(valid_str(g.pctl_dbfname))
    param.dbname = g.pctl_dbfname;
    
  param.mfdbname = NULL;
  if(valid_str(g.pctlmf_dbfname))
    param.mfdbname = g.pctlmf_dbfname;

  param.mode = g.dbfile_mode;
  param.extent_size = g.dbextent_size;
  param.reclen = const_pce_data_size();
  param.memfile_minsize = g.memfile_minsize;
  param.softlimit = g.pctl_maxsize_soft;
  param.hardlimit = g.pctl_maxsize_hard;
  param.mf_softlimit_mb = g.pctl_maxmem_soft;
  param.mf_hardlimit_mb = g.pctl_maxmem_hard;

  if(g.feedmode == FEEDMODE_MASTER)
    num_active_channels = NPCAST_NUM_CHANNELS;
  else
    num_active_channels = g.num_slavenbs_readers;

  pctl = epctl_open(num_active_channels, &param);
  if(pctl == NULL){
    log_err("Error initializing product control list.");
    return(-1);
  }else
    g.pctl = pctl;

  return(status);
}

void kill_pctl(void){

  if(g.pctl != NULL){
    epctl_close(g.pctl);
     g.pctl = NULL;
  }

  log_info("Closed pctl db.");
}

void sync_pctl(void){
  /*
   * To be called periodically.
   */
  (void)epctl_sync(g.pctl);
}

void rept_pctl_quota(void){
  /*
   * To be called periodically.
   */
  epctl_report_status(g.pctl);
}

static int process_pctl_element(struct pctl_element_st *pce){
  /*
   * The readers (via the frame procesor) insert products in the processor
   * queue even if there have been errors receiving the frames (e.g., missing
   * frames or other errors). This is done so that the processor can
   * keep track of the statistics and also update the received files db,
   * which is used to handle retransmisions. The f_complete flag of pce
   * will be 1 only if the readers received all the frames without
   * errors. Otherwise the product is not complete.
   *
   * Returns:
   *
   *    0 if the file was processed with no errors.
   *    1 if there is a system (IO) error when assembling the files
   *      (errors from mcat_frames)
   *
   * If there were missing frames, the function does what it has to do,
   * and returns 0.
   */  
  int status = 0;
  int orig_prod_status;
  unsigned int seqnumber;

  seqnumber = pce->seq_number;

  /*
   * The nbspdb_flag, which indicates whether a record of the product
   * sould be inserted in the nbsp db of the file received, is not initialized
   * by the readers' frame processor (to avoid the slowdown due to the db
   * lookups), so it must be initialized here.
   */
  nbspdb_flag_filter(pce);

  if(pce->f_complete == 0){
    /*
     * If the readers did not set this flag, then not all the frames have been
     * received, or there has been some error receiving the frames.
     * Update the seqnumber_fail element of the record in the db.
     */
    log_errx("Received only %d of %d fragments for %u [%s]",
	     pce->recv_fragments, pce->num_fragments, pce->seq_number,
	     pce->fname);
    log_missing(pce);
    if(pce->nbspdb_flag == 1)
      e_nbspdb_put_fail(pce->seq_number);

    return(0);
  }

  /*
   * If the product is a retransmission, check if the original is in the db.
   * (The indication of a retransmission is a non-zero value of the
   * pce->orig_seq_number.) 
   * 
   * Notice that the function
   * update_stats_products_retransmitted() has already been called
   * by sbnframeproc() in framep.c. It is possible that a retransmission
   * is not received complete, in which case it is discarded above
   * and the next block is not executed. In such cases, the number
   * or retransmisions processed plus those ignored will not equal
   * the total number of retransmissions received since the ones
   * that were incomplete are discarded.
   */
  if(pce->orig_seq_number != 0){
    log_info("%u is retransmitting %u. %s", seqnumber, pce->orig_seq_number,
	     pce->fname);

    if(pce->nbspdb_flag == 1)
      orig_prod_status = e_nbspdb_get_status(pce->orig_seq_number);
    else      
      orig_prod_status = -1;	/* same as not in db */

    update_stats_products_retransmitted_c();
    update_stats_products_retransmitted_pi(orig_prod_status);
    log_rtx(pce);

    if(orig_prod_status == 0){
      /*
       * The original was received, so we do not process it, but insert
       * in the nbspdb in case there are "retransmissions of retransmissions".
       */
      log_info("Found %u. Ignoring %u.", pce->orig_seq_number, seqnumber);
      if(pce->nbspdb_flag == 1)
	e_nbspdb_put_ok(pce->seq_number);

      return(0);
    }else if(orig_prod_status == 1){
      log_info("Recovering %u. Processing %u.", pce->orig_seq_number, seqnumber);
      update_stats_products_recovered();
    }else{
      /*
       * Not in db or error (status == -1) or db not enabled (status == -2).
       */
      log_info("Not found %u. Processing %u.",
	       pce->orig_seq_number, seqnumber);
    }
  }

  log_verbose(3, "Processing %u. ", seqnumber);

  if(mcat_frames(pce) != 0){
    log_errx("mcat() returned an error processing %u.", seqnumber);
    status = 1;
  }else
    log_verbose(1, "Created %s[%u]", pce->fname, pce->seq_number);

  /* 
   * Insert the key (the fpath) in the spool directory bookeeping array
   */
  if(status == 0)
    (void)nbsp_spooldb_insert(pce->fpath);

  /*
   * Insert the seqnumber_ok element of the record in the nbspdb, 
   * to indicate that the product has been received complete.
   */
  if((status == 0) && (pce->nbspdb_flag == 1))
      e_nbspdb_put_ok(pce->seq_number);

  /*
   * Insert the packet in the filter and server queues.
   */
  if(status == 0)
    e_export_product(pce);
   
  return(status);
}

static int process_pctl(void){
  /*
   * Waits for the pctl to have at least one complete and ready element.
   * and process the first.
   *
   * Returns 
   *	0 if it processed something
   *	1 if timed out waiting
   *   -1 if there were errors from process_pctl_element
   */
  int status;
  struct pctl_element_st *pce;

  pce = eopen_pctl_outputpce(g.pctl, g.tmout_prodcomplete_ms, &status);
  if(status == 1){
    /*
     * There is no element in the pctl queue.
     */
    log_verbose(1, "No products to process in pctl.");
    return(1);
  }else if(status == 0){
    log_verbose(1, "Processing %u.", pce->seq_number);

    status = process_pctl_element(pce);
  }

  close_pctl_outputpce(g.pctl);

  update_stats_products(status);
  return(status);
}

static int mcat_frames(struct pctl_element_st *pce){
  /*
   * Returns:
   *
   *     0 if no errors
   *     1 if unzip returned an error
   *     2 some inconsistency error when reading 
   *       (which cannot happen now reading from the memfile).
   *    -1 if a system (IO) error 
   */
  int fd_to = -1;
  struct memfile_st *mf = NULL;
  int status = 0;
  int unz_status = 0;
  int i;
  int n;
  int numfragments;
  char frdata[MAX_FRDATA_SIZE];
  int frdata_maxsize = MAX_FRDATA_SIZE;
  char *pdata;
  int pdata_size;
  int f_compressed;
  int saveunz_flag;

  numfragments = pce->num_fragments;
  saveunz_flag = pce->saveunz_flag;
 
  fd_to = e_open_product_file(pce);
  if(fd_to == -1)
    return(-1);

  /*
   * If the saveunz_flag is not set, then the memfile is saved as is. 
   * Otherwise, we must process each individual frame stored in the
   * memframe file. (See msave_frame() in framep.c)
   */
  if(saveunz_flag == 0){
    mf = pce->mf;
    goto end;
  }else{
    mf = reopen_nbspproc_memfile();
    if(mf == NULL){
      log_err("Could not create memfile.");
      close(fd_to);
      return(-1);
    }
  }	

  for(i = 0; i <= numfragments - 1; ++i){
    pdata_size = get_memframe(pce->mf, &pdata, &f_compressed);
    if(pdata_size <= 0){
      log_errx("Error reading frame %d from memfile %u.", i, pce->seq_number);
      status = 2;
      break;
    }

    /*
     * The frame must be uncompressed only if the data is compressed.
     */
    if(f_compressed == 1){
      n = pdata_size;
      pdata_size = frdata_maxsize;
      unz_status = unz(frdata, &pdata_size, pdata, n);
      if(unz_status != 0){
	status = 1;
	log_errx("Unzip error %d reading memframe %u.%d [%s]", unz_status,
		 pce->seq_number, i, pce->fname);
      }else
	pdata = frdata;
    }

    if(status == 0){
      if(write_memfile(mf, pdata, pdata_size) == -1){
	status = -1;
	log_err2("Cannot write", pce->fname);
      }
    }

    if(status != 0)
      break;
  }

 end:

  if(status == 0){
    if((pce->save_format == SAVEFORMAT_NO_CCB) && (pce->f_has_ccb == 1))
      status = save_memfile_skip(fd_to, mf, CCB_SIZE);
    else
      status = save_memfile(fd_to, mf);

    if(status != 0)
      log_err2("Cannot write", pce->fname);
  }

  close(fd_to);

  /*
   * The nbspproc memfile (pctl->outputmf) is not closed.
   */

  return(status);
}

static struct memfile_st *reopen_nbspproc_memfile(void){

  struct memfile_st *mf;

  mf = reopen_pctl_outputmf(g.pctl);

  return(mf);
}

static void nbsp_periodic(void){
  /*
   * Report the state of queues.
   */
  e_report_qstate();
}

static int e_export_product(struct pctl_element_st *pce){
  /*
   * The format of the data is:
   *
   *  byte[0-3]: pdh_product_seq_number
   *  byte[4]: psh_product_type
   *  byte[5]: psh_product_category
   *  byte[6]: psh_product_code
   *  byte[7]: np_channel_index
   *  byte[8-(8+FNAME_SIZE)]: fname (including final '\0')
   *  full path of file (including the trailing '\0' and padding to
   *                     fit the db queue record length if necessary)
   *
   * This is taken care of by the nbsfp_pack functions (packfp.[ch])
   */ 
  int status1 = 0;
  int status2 = 0;

  status1 = nbsfp_pack_fpath(&gpacketinfo, pce);
  if(status1 != 0){
    log_err2("Not queueing", pce->fpath); 
    
    return(status1);
  }

  if(filterq_regex_match(pce->fname) == 0)
    status1 = e_nbspq_snd_filter(gpacketinfo.packet, gpacketinfo.packet_size);
  else
    log_verbose(1, "Filter queue regex is rejecting %s.", pce->fname);

  if(serverq_regex_match(pce->fname) == 0)
    status2 = e_nbspq_snd_server(gpacketinfo.packet, gpacketinfo.packet_size);

  if(status1 == 0)
    status1 = status2;
  
  return(status1);
}

static void nbspdb_flag_filter(struct pctl_element_st *pce){

  int flag = 0;		/* default is to _not_ insert any */

  if(nbspdb_regex_match(pce->fname) == 0)
    flag = 1;

  pce->nbspdb_flag = flag;
}

static void log_missing(struct pctl_element_st *pce){

  FILE *f = NULL;

  f = fopen(g.missinglogfile, "a");
  if(f == NULL){
    log_err2("Error opening", g.missinglogfile);
    return;
  }

  write_log_pce(f, pce);

  fclose(f);
}

static void log_rtx(struct pctl_element_st *pce){

  FILE *f = NULL;

  f = fopen(g.rtxlogfile, "a");
  if(f == NULL){
    log_err2("Error opening", g.rtxlogfile);
    return;
  }

  write_log_pce(f, pce);

  fclose(f);
}

static void write_log_pce(FILE *f, struct pctl_element_st *pce){

  fprintf(f, "%" PRIuMAX " %u %d %d %d %d %s\n",
	  (uintmax_t)pce->ts.tv_sec,
	  pce->seq_number, pce->psh_product_type, pce->psh_product_category,
	  pce->psh_product_code, pce->np_channel_index, pce->fbasename);
}  

/*
 * The higher level spooldb functions.
 */
static int nbsp_spooldb_open(void){

  int keysize;
  int status = 0;

  g.spooldb = NULL;

  if(g.spooldbslots == 0){
    /*
     * The spooldb bookeeping is disabled. The spool directory will continue
     * to grow, and the hourly cron script must handle the cleaning job
     * of the spool directory.
     */
    return(0);
  }

  if(g.spooldbslots < SPOOLDB_SLOTS_MIN)
    g.spooldbslots = SPOOLDB_SLOTS_MIN;

  keysize = const_fpath_maxsize() + 1;

  g.spooldb = spooldb_init(g.spooldbslots, keysize, nbsp_spooldb_delete_fpath);
  if(g.spooldb == NULL){
    log_err("Could not create spooldb.");
    return(-1);
  }

  status = file_exists(g.spooldbfname);
  if(status == 1){
    /*
     * The file does not exist.
     */
    return(0);
  }

  if(status == 0)
    status = spooldb_read(g.spooldb, g.spooldbfname);

  if(status == -1)
    log_err_read(g.spooldbfname);
  else if(status == 1)
    log_errx("Saved spooldb file is inconsistent with configuration.");

  if(status != 0){
    spooldb_destroy(g.spooldb);
    g.spooldb = NULL;
  }

  return(status);
}

static void nbsp_spooldb_close(void){

  int status = 0;

  if(g.spooldb == NULL){
    /*
     * The spooldb was not enabled.
     */
    return;
  }

  status = spooldb_write(g.spooldb, g.spooldbfname);
  if(status != 0)
    log_err_write(g.spooldbfname);

  spooldb_destroy(g.spooldb);
  g.spooldb = NULL;
}

static int nbsp_spooldb_insert(char *fpath){

  int status = 0;

  if(g.spooldb == NULL){
    /*
     * The spooldb was not enabled.
     */
    return(0);
  }

  status = spooldb_insert(g.spooldb, fpath);
  if(status != 0){
    /*
     * An error here means that the file occupying the slot where the
     * new key was inserted could not be deleted.
     */
    log_err("Could not delete old file from spool directory.");
  }

  return(status);
}

static int nbsp_spooldb_delete_fpath(char *fpath){

  int status = 0;

  if(fpath[0] == '\0'){
    /*
     * Unoccupied slot.
     */
    return(0);
  }

  status = file_delete(fpath);
  if(status != 0)
    log_err2("Could not delete", fpath);

  return(status);
}

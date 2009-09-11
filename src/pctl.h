/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef PCTL_H
#define PCTL_H

#include <time.h>
#include <limits.h>
#include <pthread.h>
#include <db.h>
#include "mfile.h"
#include "sbn.h"	/* MAX_CTRLHDR_SIZE */
#include "const.h"

struct pctl_element_st {
  int f_complete;	    /* 0 = missing frames */
  int num_fragments;	    /* number of fragments this product was divided */
  int recv_fragments;	    /* number of fragments received */
  unsigned int seq_number;
  unsigned int orig_seq_number; /* non-zero only in retransmissions */
  int sbn_datstream;
  int psh_product_type;
  int psh_product_category;
  int psh_product_code;
  int np_channel_index;	    /* same as in the sbnf */
  struct timespec ts;	    /* receive time */ 
  struct timespec tsqueue;  /* time of insertion in queue (set in pctldb.c) */
  int save_format;	    /* how to treat the ccb hdr of the product file */
  int saveunz_flag;      /* 1 if the compresed files should be uncompressed */
  int rtxdb_flag;	    /* whether it should be put in the rtx db */
  struct memfile_st *mf;    /* memory file of the frames */
  char ctrlhdr[MAX_CTRLHDR_SIZE];         /* our copy of sbn_frame->ctrlhdr */
  int ctrlhdr_size;
  int f_has_ccb;
                            /* extracted from first line of file, NULL term. */
  char wmo_id[WMO_ID_SIZE + 1];            /* TTAAii */	    
  char wmo_station[WMO_STATION_SIZE + 1];	    
  char wmo_time[WMO_TIME_SIZE + 1];         
	                    /* extracted from second line of file */
  char wmo_awips[WMO_AWIPS_SIZE + 1];        /* a true awips */
  char wmo_notawips[WMO_NOTAWIPS_SIZE + 1];  /* not awips: GRIB, BUFR, etc */
                            /* built from the above (all with '\0') */
  char fname[FNAME_SIZE + 1];		   /* root name without seq key */
  char fbasename[FBASENAME_SIZE + 1];	   /* base name (includes seq key) */
  int fpath_size;			   /* allocated size for fpath[] */
  char fpath[1];			   /* full path */
};

struct pctldb_st {
  DB *dbp;		/* keeps the pce */
  DB *mfdbp;		/* keeps the mfiles */
  uint32_t pce_size;		/* total allocated size of a pce */
  uint32_t pce_data_size;	/* what is used */
  uint32_t n;
  uint32_t nmax;	/* from db: 0xffffffff */
  size_t mf_total_size;		/* in bytes */
  struct timespec last_tsqueue;	/* tsqueue of last pce inserted */ 
  uint32_t softlimit;
  uint32_t hardlimit;
  size_t mf_softlimit_mb;
  size_t mf_hardlimit_mb;
  pthread_cond_t  cond;
  pthread_mutex_t mutex;
};

/* for keeping quota limit status */
struct queue_status_st {
  int current;
  int last;
};

/*
 * for passing the configuration parameters to the pctldb
 */
struct pctldb_param_st {
  DB_ENV *dbenv;
  char *dbname;		/* main (pce) db file */
  char *mfdbname;	/* mf db file */
  int mode;
  int extent_size;
  uint32_t reclen;
  size_t memfile_minsize;
  uint32_t softlimit;
  uint32_t hardlimit;
  size_t mf_softlimit_mb;
  size_t mf_hardlimit_mb;
};

struct pctl_st {
  struct pctldb_st      *pctldb;     /* access by readers and processor */
  struct pctl_element_st **activepce; /* access only by readers */
  int num_activechannels;	     /* one for each noaaport channel */
  struct memfile_st *outputmf;   /* working mfile for assembling frames */
  struct pctl_element_st *pce;   /* working pce for processor */
  struct queue_status_st status;
};

struct pctl_st *epctl_open(int numchannels, struct pctldb_param_st *param);
void epctl_close(struct pctl_st *pctl);
int epctl_send(struct pctl_st *pctl, int channel_index);
int epctl_rcv(struct pctl_st *pctl, int timeout_ms);
int epctl_sync(struct pctl_st *pctl);

struct pctl_element_st *eopen_pctl_outputpce(struct pctl_st *pctl,
					    int timeout_ms,
					    int *status);
void close_pctl_outputpce(struct pctl_st *pctl);
struct pctl_element_st *get_pctl_outputpce(struct pctl_st *pctl);
struct memfile_st *reopen_pctl_outputmf(struct pctl_st *pctl);

struct pctl_element_st *open_pctl_activepce(struct pctl_st *pctl,
					    int channel_index);
void close_pctl_activepce(struct pctl_st *pctl, int channel_index);
int esend_pctl_activepce(struct pctl_st *pctl, int channel_index);
struct pctl_element_st *get_pctl_activepce(struct pctl_st *pctl,
					   int channel_index,
					   unsigned int seqnumber);
int update_activepce_framecounter(struct pctl_st *pctl,
				  int channel_index,
				  unsigned int seqnumber,
				  int block_number);
int complete_activepce(struct pctl_st *pctl,
		       int channel_index,
		       unsigned int seqnumber);

/*
 * Private functions to pctl itself
 */

/*
 * Create/destroy a pce
 */
int const_pce_size(void);
int const_pce_data_size(void);
struct pctl_element_st *create_pce(size_t memfile_size);
void destroy_pce(struct pctl_element_st *pce);
int open_pce(struct pctl_element_st *pce);
void close_pce(struct pctl_element_st *pce);

/*
 * pctldb
 */
int pctldb_open(struct pctldb_st **pctldb, struct pctldb_param_st *param,
		int *dberror);
int pctldb_close(struct pctldb_st *pctldb, int *dberror);
int pctldb_snd(struct pctldb_st *pctldb, void *p, uint32_t size, int *dberror);
int pctldb_rcv(struct pctldb_st *ptcldb, void *p, uint32_t *size, int *dberror);
int pctldb_send_pce(struct pctldb_st *pctldb,
		    struct pctl_element_st *pce,
		    int *dberror);
int pctldb_rcv_pce(struct pctldb_st *pctldb,
		   struct pctl_element_st *pce,
		   int timeout_ms,
		   int *dberror);
int pctldb_sync(struct pctldb_st *pctldb, int *dberror);
void epctl_report_status(struct pctl_st *pctl);

/*
 * Functions for storing and getting the mfile from the pctldb.
 * (pctlmfdb.c)
 */
int pctlmfdb_open(struct pctldb_st *pctldb,
		  struct pctldb_param_st *param, int *dberror);
int pctlmfdb_close(struct pctldb_st *pctldb, int *dberror);
int pctlmfdb_snd(struct pctldb_st *pctldb, struct timespec *ts,
		 struct memfile_st *mf, int *dberror);
int pctlmfdb_rcv(struct pctldb_st *pctldb, struct timespec *ts,
		 struct memfile_st *mf, int *dberror);

#endif

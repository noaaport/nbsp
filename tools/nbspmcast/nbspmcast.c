/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 */

/*
 * Usage: npmcast [-b] [-C] [-m] [-r] [-u frame_rate ] \
 *                [-a mcast_addr] [-p mcast_port] \
 *                [-i ifname | -I ifip] \
 *                [-s prod_seq_num] [-f sbn_seq_num] \
 *                <fpath>
 * (The "sbn_seq_num" is the start of the sequence number for each frame.)
 *
 * -b => background
 * -C => print configuration (and exit)
 * -m => setsockopt mcast loop off flag
 * -r => radar (nids) file; default is nwstg
 *       (also sets the multicast ip and port)
 * -u => frames per ms (esentially sets the argument to usleep between frames)
 *        -1 => use the default (10) frames/ms;
 *              the corresponding useconds for usleep is 0.1 ms or 100 usecs;
 *              corresponds to about 4KB * 10 * 1000/s = 40 MB/s.
 *        0  => use usleep with useconds = 0 (the default program setting)
 *
 * Exit codes:
 *   0 => no errors
 *   1 => some error
 */
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>	/* PRIu */
#include <libgen.h>	/* basename */
#include <sys/types.h>  /* gai_sterror */
#include <sys/socket.h> /* gai_sterror */
#include <netdb.h>      /* gai_sterror */
#include "err.h"
#include "stoi.h"
#include "sbnpack_usleep.h"
#include "seqnum.h"
#include "sbnpack.h"
#include "mcast.h"

/*
 * The four original channels:
 * 1 ncep/nwstg, 2 goes/nesdis, 3 ncep/nwstg2, 4 oconus imagery/model
 */
#define MCAST_ADDR_1 "224.0.1.1"
#define MCAST_ADDR_2 "224.0.1.2"
#define MCAST_ADDR_3 "224.0.1.3"
#define MCAST_ADDR_4 "224.0.1.4"
#define MCAST_PORT_1 "1201"
#define MCAST_PORT_2 "1202"
#define MCAST_PORT_3 "1203"
#define MCAST_PORT_4 "1204"

/* defaults */
#define DEF_MCAST_ADDR MCAST_ADDR_1
#define DEF_MCAST_PORT MCAST_PORT_1
#define DEF_IFNAME NULL
#define DEF_IFIP NULL
#define DEF_PROD_SEQ_NUM 0   /* the default will be the msecs since midnight */
#define DEF_SBN_SEQ_NUM (UINT32_MAX/4 - 2)
#define DEF_MCAST_TTL	-1  /* use default: mcast only to the local network */
#define DEF_MCAST_LOOP -1   /* use default; on */
#define DEF_FRAMES_PER_MS 0 /* make usleep do nothing */
/* #define DEF_FRAMES_PER_MS -1 use the default setting of the library */

struct {
  int opt_background;		/* -b */
  int opt_C;			/* -C => print configuration */
  int opt_r;			/* -r => radar (nids) file; default is nwstg */
  int opt_frames_per_ms;	/* -u frames_per_ms */
  char *opt_mcast_addr;		/* -a */ 
  char *opt_mcast_port;		/* -p */
  char *opt_ifname;		/* -i interface_name (e.g., eth1) */
  char *opt_ifip;		/* -I interface_ip (e.g., 192.168.10.1) */
  uint32_t opt_prod_seq_num;	/* -s */
  uint32_t opt_sbn_seq_num;	/* -f */
  int  opt_mcast_ttl;		/* -t mcast_ttl */
  int  opt_mcast_loop_off;	/* -m => setsockopt mcast loop flag off */
  char *opt_fpath;
  /* variables */
  char *stdin_fpath;
  struct sbnpack_st *sbnpack;
  int sfd;			/* socket fd */
  void *sa;			/* struct sockaddr *sa; */
  socklen_t sa_len;
} g = {0, 0, 0, DEF_FRAMES_PER_MS, DEF_MCAST_ADDR, DEF_MCAST_PORT, NULL, NULL,
       DEF_PROD_SEQ_NUM, DEF_SBN_SEQ_NUM, DEF_MCAST_TTL, DEF_MCAST_LOOP, NULL,
       NULL, NULL, -1, NULL, 0};

/* general functions */
static int process_stdin(void);
static int process_file(void);
static void print_conf(void);
static void init(void);
static void cleanup(void);

/* mcast sento functions */
static int sendto_sbnpack(int sfd, struct sbnpack_st *sbnpack,
			  struct sockaddr *sa, socklen_t sa_len);
static int sendto_sbnpack_frame(int fd,
				struct sbnpack_st *sbnpack, int findex,
				struct sockaddr *sa, socklen_t sa_len);

static void init(void) {

  int gai_code;

  init_sendto_sbnpack_usleep(g.opt_frames_per_ms);

  /*
   * Open the file descriptor for sending multicast messages
   */
  g.sfd = mcast_snd(g.opt_mcast_addr, g.opt_mcast_port,
		    g.opt_ifname, g.opt_ifip,
		    g.opt_mcast_ttl, g.opt_mcast_loop_off,
		    &g.sa, &g.sa_len, &gai_code);

  if(g.sfd < 0){
    if(gai_code == 0)
      log_err(1, "%s", "Error from mcast_snd()");
    else
      log_errx(1, "%s: %s", "Error from mcast_snd", gai_strerror(gai_code));
  }

  g.sbnpack = create_sbnpack();
  if(g.sbnpack == NULL)
    log_err(1, "%s", "Error from create_sbnpack");
}

static void cleanup(void){

  if(g.stdin_fpath != NULL)
    free(g.stdin_fpath);
  
  /* free the contents of the sbnpack; and the sbnpack itself */
  if(g.sbnpack != NULL) {
    free_sbnpack(g.sbnpack);
    free(g.sbnpack);
  }

  /* the multicast descriptor */
  if(g.sfd != -1)
    close(g.sfd);

  if(g.sa != NULL)
    free(g.sa);
  
  return;
}

static void print_conf(void){

  fprintf(stdout, "%s: %d\n", "opt_background", g.opt_background);
  fprintf(stdout, "%s: %d\n", "opt_r", g.opt_r);

  fprintf(stdout, "%s: %d\n", "opt_frames_per_ms", g.opt_frames_per_ms);
    
  fprintf(stdout, "%s: %s\n", "opt_mcast_addr", g.opt_mcast_addr);
  fprintf(stdout, "%s: %s\n", "opt_mcast_port", g.opt_mcast_port);
  fprintf(stdout, "%s: %s\n", "opt_ifname", g.opt_ifname);
  fprintf(stdout, "%s: %s\n", "opt_ifip", g.opt_ifip);

  fprintf(stdout, "%s: %" PRIu32 "\n", "opt_prod_seq_num", g.opt_prod_seq_num);
  fprintf(stdout, "%s: %" PRIu32 "\n", "opt_sbn_seq_num", g.opt_sbn_seq_num);

  fprintf(stdout, "%s: %" PRIu8 "\n", "opt_mcast_ttl", g.opt_mcast_ttl);
  fprintf(stdout, "%s: %" PRIu8 "\n", "opt_mcast_loop_off",
	  g.opt_mcast_loop_off);

  if(g.opt_fpath != NULL)
    fprintf(stdout, "%s: %s\n", "fpath", g.opt_fpath);
}

int main(int argc, char **argv){

  char *optstr = "bCmru:a:p:i:I:s:f:t:";
  char *usage = "npmcast [-b] [-C] [-m] [-r] [-u frames_per_ms] \
                 [-a mcast_addr] [-p mcast_port] \
                 [-i ifname | -I ifip] \
                 [-s prod_seq_num] [-f sbn_seq_num] \
		 [-t mcast_ttl] \
                 <fpath>";
  int status = 0;
  int opt_iI = 0;	/* i and I together is a conflict */
  int c;

  set_progname(basename(argv[0]));

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'b':
      g.opt_background = 1;
      break;
    case 'C':
      g.opt_C = 1;
      break;
    case 'm':
      g.opt_mcast_loop_off = 0;
      break;
    case 'r':
      g.opt_r = 1;	/* file is radar (nids) */
      g.opt_mcast_addr = MCAST_ADDR_3;
      g.opt_mcast_port = MCAST_PORT_3;
      break;
    case 'u':
      if(strto_int(optarg, &g.opt_frames_per_ms) != 0) {
	log_errx(1, "Invalid argument to -u option: %s", optarg);
      }
      break;
    case 'a':
      g.opt_mcast_addr = optarg;
      break;
    case 'p':
      g.opt_mcast_port = optarg;
      break;
    case 'i':
      g.opt_ifname = optarg;
      ++opt_iI;
      break;
    case 'I':
      g.opt_ifip = optarg;
      ++opt_iI;
      break;
    case 's':
      /*
	if(sscanf(optarg, "%" PRIu32, &g.opt_prod_seq_num) != 1){
	  log_errx(1, "Invalid argument to -s option: %s", optarg);
        }
      */
      if(strto_u32(optarg, &g.opt_prod_seq_num) != 0) {
	log_errx(1, "Invalid argument to -s option: %s", optarg);
      }
      break;
    case 'f':
      /*
	if(sscanf(optarg, "%" PRIu32, &g.opt_sbn_seq_num) != 1){
	  log_errx(1, "Invalid argument to -f option: %s", optarg);
	}
      */
      if(strto_u32(optarg, &g.opt_sbn_seq_num) != 0) {
	log_errx(1, "Invalid argument to -f option: %s", optarg);
      }      
      break;
    case 't':
      /*
	if(sscanf(optarg, "%d", &g.opt_mcast_ttl) != 1){
	  log_errx(1, "Invalid argument to -t option: %s", optarg);
	}
      */
      if(strto_int(optarg, &g.opt_mcast_ttl) != 0) {
	log_errx(1, "Invalid argument to -t option: %s", optarg);
      }            
      break;
    default:
      log_info(usage);
      exit(0);
      break;
    }
  }

  if(g.opt_background == 1)
    set_usesyslog();

  if(opt_iI > 1)
    log_errx(1, "Invalid combination of options: i and I.");

  if(g.opt_prod_seq_num == DEF_PROD_SEQ_NUM)
    g.opt_prod_seq_num = get_seqnum_start();

  if(g.opt_C == 1){
    print_conf();
    exit(0);
  }

  atexit(cleanup);
  init();

  if(optind > argc - 1) {
    status = process_stdin();
  } else {
    while(optind <= argc - 1) {
      g.opt_fpath = argv[optind++];
      ++g.opt_prod_seq_num;
      if(process_file() != 0) {
	status = 1;
	log_errx(0, "%s", "Error processing", g.opt_fpath);
      }
    }
  }
  
  return(status != 0 ? 1 : 0);
}

static int process_stdin(void)  {
  
  size_t fpath_size = 0;
  ssize_t fpath_len;
  int status = 0;

  while(1) {
    fpath_len = getline(&g.stdin_fpath, &fpath_size, stdin);
    
    if(fpath_len == -1) {
      break;
    }
    
    if(g.stdin_fpath[fpath_len - 1] == '\n'){
      g.stdin_fpath[fpath_len - 1] = '\0';
      --fpath_len;
    }

    /* blank lines */
    if(fpath_len == 0)
      continue;

    g.opt_fpath = g.stdin_fpath;
    ++g.opt_prod_seq_num;

    if(process_file() != 0) {
      status = 1;
      log_err(0, "%s: %s", "Error processing", g.opt_fpath);
    }
  }

  if(fpath_len == -1) {
    if(ferror(stdin) != 0)
      log_err(1, "%s", "Error from getline");
  }
  
  return(status);
}

static int process_file(void){

  int status = 0;

  status = init_sbnpack(g.sbnpack,
			g.opt_fpath,
			g.opt_prod_seq_num,
			g.opt_sbn_seq_num,
			g.opt_r);
  if(status == -1)
    log_err(0, "%s", "Error from create_sbnpack");
  else if(status == 1)
    log_errx(0, "%s", "File too large");

  if(status == 0) {
    status = sendto_sbnpack(g.sfd, g.sbnpack, g.sa, g.sa_len);
    if(status != 0) {
      log_err(0, "%s", "Error from sendto_sbnpack");
    }
  }
  
  return(status);
}

static int sendto_sbnpack(int sfd, struct sbnpack_st *sbnpack,
		   struct sockaddr *sa, socklen_t sa_len){

  int nframes;
  int i;
  int status = 0;
  
  nframes = sbnpack->nframes;
  
  for(i = 0; i < nframes; ++i){
    status = sendto_sbnpack_frame(sfd, sbnpack, i, sa, sa_len);
    if(status != 0)
      break;

    sendto_sbnpack_usleep();
  }

  return(status);
}

static int sendto_sbnpack_frame(int fd,
				struct sbnpack_st *sbnpack, int findex,
				struct sockaddr *sa, socklen_t sa_len){
  int framesize;
  ssize_t n;
  int status = 0;

  assert(findex < sbnpack->nframes);

  framesize = sbnpack->sbnpack_frame[findex].datablock_size +
    sbnpack->sbnpack_frame[findex].header_size;
  
  n = sendto(fd, sbnpack->sbnpack_frame[findex].frame, framesize,
	     0, sa, sa_len);
  if(n == -1)
    status = -1;
  else if(n != framesize)
    status = 1;
  
  return(status);
}

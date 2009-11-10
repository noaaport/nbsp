/*
 * Copyright (c) 2005-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <syslog.h>
#include <libgen.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include "libconnth/libconn.h"
#include "access.h"
#include "readn.h"
#include "err.h"
#include "init.h"
#include "signal.h"
#include "globals.h"
#include "packfpu.h"
#include "util.h"
#include "appdata.h"
#include "emwin.h"
#include "nbspre.h"
#include "nbs1.h"
#include "nbspq.h"
#include "sfilter.h"
#include "pfilter.h"
#include "server.h"
#include "server_priv.h"

/* This is used in lookup_client_options() */
#define CLIENTOPTION_SEP_CHAR ','

/*
 * The server provides its own allocation for the packetinfo->packet
 * that is retrieved from the queue, to avoid having the qdb functions
 * to allocate memory for it for every packet.
 */
static struct pfilter_st *gemwinfilterp = NULL;
static struct sfilterp_st *gnetfilterp = NULL;

static struct packet_info_st gpacketinfo = {0, 0, 0, 0, 0,
					    NULL, NULL, NULL, 0};

static int greload_server_filters = 0;	    
static pthread_mutex_t greload_server_filters_mutex =
		PTHREAD_MUTEX_INITIALIZER;
static int get_reload_server_filters_flag(void);

static int greport_client_connections = 0;
static pthread_mutex_t greport_client_connections_mutex =
	PTHREAD_MUTEX_INITIALIZER;
static int get_server_state_flag(void);
static void report_client_connections(void);
static void print_client_connections(FILE *fp);

static int open_server(void);
static void close_server(void *arg);

static void init_server_globals(void);
static int init_server_conn(void);
static void close_server_conn(void);
static int open_server_filters(void);
static void close_server_filters(void);

static int init_unpack_packet_pool(void);
static void cleanup_unpack_packet_pool(void);

static int create_server_thread(void);
static void *server_main(void*);
static int server_loop(void);
static int process_server_queue(void);
static int handle_client(struct conn_table_st *ct, int i);
static void handle_client_hup(struct conn_table_st *ct, int i, int condition);

/*
 * The process_connections() function is executed periodically,
 * but we feel there is no need to let the period be a run-time
 * configuration option. In case we later decide to do that, we leave
 * here some place holders for the flag functions that can be used in
 * per.c, similar to the other flags of the server.
 *
 * static int gprocess_connections = 0;
 * static pthread_mutex_t gprocess_connections_mutex =
 * PTHREAD_MUTEX_INITIALIZER;
 */
#define SERVER_PROCESS_CONNECTIONS_PERIOD_SECS 1
static time_t gprocess_connections_time = 0;	/* next time to process */
static int get_process_connections_flag(void);
static void process_connections(void);
static void process_unidentified_connections(void);
static void process_finished_connections(void);
static void process_dirty_connections(void);

static int server_send_client_queues(struct packet_info_st *packetinfo);
static int exec_emwin_filter(struct packet_info_st *packetinfo,
			     char *fpathout, size_t fpathout_size,
			     char *emwinfname, size_t emwinfname_size);
static int e_emwin_pfilter_read(struct pfilter_st *filterp,
				void *buffer, size_t size);
static void close_emwin_filter(void);
static int open_emwin_filter(void);
static int match_client_protocol(struct conn_table_st *ct, int i);

static void spawn_client_threads(void);
static void lookup_client_options(struct conn_element_st *ce,
				  char *clientoptions);

int spawn_server(void){
  /*
   * This is the entry to the server module. It is called by main after
   * forking to spawn the main server thread.
   */
  int status = 0;

  status = create_server_thread();

  return(status);
}

void kill_server_thread(void){
  /*
   * This "joins" the server thread. The client threads are "joined"
   * when the conn table is destroyed.
   */
  
  if(g.f_server_thread_created == 0)
    return;

  pthread_join(g.server_thread_id, NULL);
  log_info("Finished net server.");
}

static int open_server(void){

  int status;

  init_server_globals();

  status = init_unpack_packet_pool();

  if(status == 0)
    status = init_server_conn();

  if(status == 0)
    status = open_server_filters();
   
   return(status);
}

static void close_server(void *arg __attribute__ ((unused))){

  close_server_filters();
  close_server_conn();
  cleanup_unpack_packet_pool();
}

static int create_server_thread(void){

  int status = 0;
  pthread_t t_id;
  pthread_attr_t attr;

  status = pthread_attr_init(&attr);

  /*
  if(status == 0)
    status = set_thread_priority_low(&attr);
  */

  /*
  if(status == 0)
    status = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  */

  if(status == 0)
    status = pthread_create(&t_id, &attr, server_main, NULL);

  if(status != 0){
    log_err("Cannot create server thread.");
  }else{
    g.server_thread_id = t_id;
    g.f_server_thread_created = 1;
    log_info("Spawned server thread.");
  }

  return(status);
}

static void *server_main(void *data __attribute__((unused))){

  int status;

  pthread_cleanup_push(close_server, NULL);

  status = open_server();
  if(status != 0){
    set_quit_flag();
    return(NULL);
  }

  while(get_quit_flag() == 0){
    status = server_loop();
  }

  pthread_cleanup_pop(1);

  return(NULL);
}

static int server_loop(void){

  int status = 0;

  while((status == 0) && (get_quit_flag() == 0)){
    if(get_process_connections_flag()){
      process_connections();
    }

    status = process_server_queue();

    /*
     * Report the client connections state if it has been requested.
     * This flag is set in the periodic function.
     */
    if(get_server_state_flag()){
      report_client_connections();
    }

    /*
     * The server's filters are not curently reloaded when receiving a hup
     * signal (see periodic() in per.c). We have not enabled that yet
     * because it is not straightforward how to proceed if the new filter
     * has an error (i.e., kill the server or continue trying to reload
     * the filter).
     */     
    if(get_reload_server_filters_flag()){
      /*
       *   close_server_filters();
       *   status = open_server_filters();
       *   while(status != 0)
       *     status = open_server_filters();
       */
      log_info("%s", "Servers filters are not reloaded.");
    }
  }
    
  return(status);
}

static void init_server_globals(void){
  
  if((g.servertype & PROTOCOL_NBS1) != 0)
    g.f_nbs1server_enabled = 1;

  if((g.servertype & PROTOCOL_NBS2) != 0)
    g.f_nbs2server_enabled = 1;

   if((g.servertype & PROTOCOL_EMWIN) != 0)
    g.f_emwinserver_enabled = 1;
}

static int init_server_conn(void){
  /*
   * Opens the socket for network clients (via the libconn library).
   */
  int status = 0;
  int gai_code;
  int backlog = g.server_listen_backlog;

  if(valid_str(g.serverport) == 0){
    log_errx("Server port not set.");
    return(1);
  }

  g.server_fd = tcp_server_open_conn(g.servername, g.serverport,
				     backlog, g.server_so_sndbuf, -1,
				     NULL, &gai_code);
  if(g.server_fd == -1){
    if(gai_code != 0)
      log_errx("Cannot open listening port. %s", gai_strerror(gai_code));
    else
      log_err("Cannot open listening port.");

    return(-1);
  }

  if(fcntl(g.server_fd, F_SETFD, FD_CLOEXEC) == -1){
    log_err("Cannot set cloexec flag on server fd.");
    close(g.server_fd);
    g.server_fd = -1;
    return(-1);
  }

  g.ct = conn_table_create(g.server_maxclients,
			   handle_client,
			   handle_client_hup,
			   client_allow_nconn);
  if(g.ct == NULL){
    log_err("Cannot init server.");
    return(-1);
  }

  /* 
   * This server does not (yet) have a control socket. The server entry
   * is added with pid = 0, ip = NULL, name = NULL,
   * and the options (timeout_ms, timeout_retry, reconnect_sleep,
   * reconnect_retry) 0.
   */
  status = conn_table_add_element(g.ct, g.server_fd,
				  CONN_TYPE_SERVER_NET, 0, NULL, NULL,
				  0, 0, 0, 0);

  if(status != 0)
    log_err("Cannot init server.");
  else
    log_info("Server initialized.");

  return(status);
}

static void close_server_conn(void){
  /*
   * When each g.ct->ce element is released, the corresponding client thread
   * is also killed by the callback function ce->thkillproc (defined in
   * serverc.c).
   */

  if(g.ct != NULL){
    conn_table_destroy(g.ct);
    g.ct = NULL;
  }

  if(g.server_fd != -1){
    close(g.server_fd);
    g.server_fd = -1;
  }
}

static int open_server_filters(void){
  /*
   * Open the two sfilters used by the server: the emwinfilter,
   * and the netfilter (for controlling allow/deny access).
   */
  int status = 0;

  if(g.f_emwinserver_enabled == 1){
      if(valid_str(g.emwinfilter) == 0){
	log_errx("No emwin filter defined.");
	return(1);
      }

      if(valid_str(g.emwinfilter_fifo) == 0){
	log_errx("No emwin filter fifo defined.");
	return(1);
      }

      if(open_emwin_filter() != 0){
	log_err("Cannot initalize server emwin filter.");
	return(-1);
      }
  }

  if(g.netfilter_enable == 0)
    return(0);

  if(valid_str(g.netfilter) == 0){
    log_errx("No netfilter defined.");
    status = 1;
  }

  if(status == 0){
    gnetfilterp = open_sfilter(g.netfilter);
    if(gnetfilterp == NULL){
      log_err("Cannot initalize server access filter.");
      status = -1;
    }
  }

  /*
   * Initialize the filter scripts that the server will evaluate.
   */
  if(status == 0)
    status = init_sfilter_script(gnetfilterp);

  if(status != 0){
    close_emwin_filter();
    if(gnetfilterp != NULL)
      close_sfilter(gnetfilterp);

    gnetfilterp = NULL;
  }

  return(status);
}

static void close_server_filters(void){

  if(gemwinfilterp != NULL)
    close_emwin_filter();

  /*
   * Send "end" command to the script.
   */
  if(gnetfilterp != NULL){
    (void)end_sfilter_script(gnetfilterp);
    close_sfilter(gnetfilterp);
    gnetfilterp = NULL;
  }
}

static int init_unpack_packet_pool(void){

  if(nbsfp_packetinfo_init(&gpacketinfo) != 0){
    log_err("Cannot initalize the server packetinfo memory pool.");
    return(-1);
  }

  return(0);
}

static void cleanup_unpack_packet_pool(void){

  nbsfp_packetinfo_cleanup(&gpacketinfo);
}

static int process_server_queue(void){

  int status = 0;
  size_t size;

  assert(gpacketinfo.packet != NULL);

  size = gpacketinfo.packet_size;  
  status = e_nbspq_rcv_server(&gpacketinfo.packet, &size);

  assert(size == gpacketinfo.packet_size);

      
  if(status == 1){
    /*
     * The queue was empty (and timedout waiting).
     */
    return(0);
  }else if(status != 0)
    return(-1);

  status = nbsfp_unpack_fpath(&gpacketinfo);

  if(status == 0){
    if(gnetfilterp != NULL)
      status = init_sfilter_input(gnetfilterp, &gpacketinfo);

    if(status == 0){
      status = server_send_client_queues(&gpacketinfo);
      if(status != 0)
	log_errx("Error queuing %s.", gpacketinfo.fpath);
      else
	log_verbose(1, "Queued %s", gpacketinfo.fpath);
    }else
      log_err("Could not initalize server filter.");
  }else if(status == -1){
    log_err("Error reading from the queue.");
  }else if(status == 1){
    log_errx("Corrupted data from queue.");
  }

  /*
   * Do not call nbsfp_pack_free() on gpacketinfo because the packet
   * pool will be reused repeatedly.
   */

  return(status);
}

static int handle_client(struct conn_table_st *ct, int i){
  /*
   * Old bb clients send some kind of login id when they connect
   * and/or some packets to keep the connection alive. To make them
   * happy, we will read some chunk of data, but discard it. NBS clients
   * should identify themselves by sending the correct message.
   * If a client connects without sending anything, it does not
   * pass through this function and we have to check his protocol
   * somewhere else, unless the server is serving all the protocols
   * in which case it does not matter. It is being checked in the
   * function process_unidentified_connections().
   */
  pid_t pid;
  int fd;
  size_t size = 512;
  char buf[512];
  ssize_t n;
  int status;
  char *ip;

  pid = conn_table_get_element_pid(ct, i);
  assert(pid == -1);		/* this server only has network clients */
  ip = conn_table_get_element_ip(ct, i);
  fd = conn_table_get_element_fd(ct, i);

  n = read1(fd, buf, size, 0, 0);
  if(n > 0){
    status = ident_client_protocol(ct, i, buf, (size_t)n);
    if(status == 2){
      /*
       * Client had already identified.
       */
      log_info("Client %s is sending data.", ip);
    } else if(status == 1){
      log_info("Cannot identify client protocol. Using default.", ip);
      /*
       * log_msg(LOG_DEBUG, "From %s: %.80s", ip, buf);
       */
    } else if(status == -1){
      log_err2("Error identifying protocol from", ip);
      (void)poll_kill_client_connection(ct, i);	
    }

    if((status == 0) || (status == 1)){
      if((status = match_client_protocol(ct, i)) != 0){
	log_errx("Client %s protocol is not being served.", ip);
	(void)poll_kill_client_connection(ct, i);
      }
    }
  } else {
    /*
     * Connection reset by peer, or disconnected. This should have been
     * caught by the libconn function poll_client_hangup()
     * [which calls the function below] so that these
     * two error messages should really not appear.
     */
    if(n < 0)
      log_err2("Unexpected app error from client", ip);
    else
      log_info("Unexpected app error. Client %s disconnected", ip);

    (void)poll_kill_client_connection(ct, i);
  }

  return(0);
}

static void handle_client_hup(struct conn_table_st *ct, int i, int condition){
  /*
   * This function is called by poll_client_hangup() in libconnth/poll.c
   * to notify the application when it has detected that a client has closed,
   * and it is about to delete the corresponding element from the table.
   * It is called if the descriptor has (POLLHUP | POLLERR | POLLNVAL) set;
   * or POLLIN in a condition that a read will return 0 or -1. 
   */
  char *ip;

  assert(condition != 0);

  ip = conn_table_get_element_ip(ct, i);
  if(condition > 0)
    log_info("Client %s disconnected", ip);
  else
    log_err2("Client", ip);
}

static void process_connections(void){
  /*
   * The function poll_loop_nowait() calls poll() with a zero
   * timeout value, so it does not block. With the options, it opens
   * any new client sockets to be non-blocking for read/write.
   */
  int status = 0;
  struct client_options_st clientopts;

  clientopts.nonblock = 1;
  clientopts.cloexec = 1;
  clientopts.write_timeout_ms = g.client_write_timeout_ms;
  clientopts.write_timeout_retry = g.client_write_timeout_retry;
  clientopts.reconnect_wait_sleep_secs = g.client_reconnect_wait_sleep_secs;
  clientopts.reconnect_wait_sleep_retry = g.client_reconnect_wait_sleep_retry;

  process_finished_connections();
  process_unidentified_connections();
  process_dirty_connections();
  spawn_client_threads();

  status = poll_loop_nowait(g.ct, &clientopts);
  if(status == -1)
    log_err("Cannot listen for connections (poll()).");
  else if(status == -2){
    log_err("Cannot accept a new connection.");
  }
}

static void process_unidentified_connections(void){

  int i;
  int numentries;
  int protocol;
  char *nameorip;
  time_t ctime;		/* when the client connected */
  time_t now;

  now = time(NULL);
  numentries = conn_table_get_numentries(g.ct);

  for(i = 0; i < numentries; ++i){
    if(conn_table_element_isnetclient(g.ct, i) == 0)
      continue;
      
    protocol = get_client_protocol(g.ct, i);
    nameorip = conn_table_get_element_nameorip(g.ct, i);
    ctime = conn_table_get_element_ctime(g.ct, i);

    if((protocol == PROTOCOL_UNKNOWN) &&
       (now > ctime + g.server_clientid_timeout_secs)){

      log_info("Client %d [%s] not identified within time limit.",
	       i, nameorip);
      log_info("Assuming %d [%s] is an emwin client.", i, nameorip);
      if(ident_client_protocol(g.ct, i, PROTOCOL_EMWIN_STR,
			       strlen(PROTOCOL_EMWIN_STR)) != 0){
	log_err2("Could not set protocol for", nameorip);
	poll_kill_client_connection(g.ct, i);
	numentries = conn_table_get_numentries(g.ct);
      }
    }
  }
}

static void process_finished_connections(void){
  /*
   * Loop through the table and remove any entry for which the
   * thread has finished (exited) for some reason.
   */
  int i;
  int numentries;

  numentries = conn_table_get_numentries(g.ct);

  for(i = 0; i < numentries; ++i){
    if(conn_table_element_isnetclient_stoped(g.ct, i) == 0)
      continue;

    log_info("Removing finished client %d from table.", i);
    poll_kill_client_connection(g.ct, i);
    numentries = conn_table_get_numentries(g.ct);
  }
}

static void process_dirty_connections(void){
  /*
   * Loop through the table and look for any entry for which the
   * connection_status flag has been set (by the client thread when
   * a write error is received). What this function does is to close
   * the socket and set the pfd[i] to -1. This is done only for
   * threads that have ben created and have not finished.
   *
   * The unlocked version of conn_element_get_connection_status_un() is used
   * since the worst that can happen is that a client that has already
   * disconected is not detected, but it will be detected in the next loop.
   */
  int i;
  int numentries;
  int connection_status;
  char *nameorip;

  numentries = conn_table_get_numentries(g.ct);

  for(i = 0; i < numentries; ++i){
    if(conn_element_isnetclient_running(&g.ct->ce[i]) == 0)
      continue;

    connection_status =  conn_element_get_connection_status_un(&g.ct->ce[i]);
    if(connection_status == 0)
      continue;

    nameorip = conn_table_get_element_nameorip(g.ct, i);
    if(g.ct->pfd[i].fd != -1){
      if(close(g.ct->pfd[i].fd) == 0){
	log_info("Closing bad client connection: %s", nameorip);
	g.ct->pfd[i].fd = -1;
      }
    }
  }
}

static void report_client_connections(void){
  /*
   * Print the ip, and queue size for each connection.
   */
  FILE *fp = NULL;
  char *fname;

  fname = g.serverstatefile;
  if(valid_str(fname)){
    fp = fopen(fname, "a");
    if(fp == NULL){
      log_err_open(fname);
    } else {
      print_client_connections(fp);
      fclose(fp);
    }
  }

  fname = g.serveractivefile;
  if(valid_str(fname)){
    fp = fopen(fname, "w");
    if(fp == NULL){
      log_err_open(fname);
    } else {
      print_client_connections(fp);
      fclose(fp);
    }
  }
}

static void print_client_connections(FILE *fp){

  int netclients;	
  int numentries;
  char *nameorip;
  int protocol;
  uint32_t queue_size;
  time_t ctime;
  int i;
  time_t now;

  now = time(NULL);
  netclients = conn_table_get_netclients(g.ct);   
  numentries = conn_table_get_numentries(g.ct);			 

  fprintf(fp, "- %u %d %d\n", (unsigned int)now, numentries, netclients);

  if(netclients == 0)
    return;

  for(i = 0; i < numentries; ++i){
    /*
     * Only the clients that already have threads created have queues.
     */
    if(conn_table_element_isnetclient_running(g.ct, i) == 0)
      continue;

    nameorip = conn_element_get_nameorip(&(g.ct->ce[i]));
    protocol = get_client_protocol(g.ct, i);
    queue_size = connqueue_n(g.ct->ce[i].cq);
    ctime = conn_table_get_element_ctime(g.ct, i);

    fprintf(fp, "  %s %d" " %" PRIu32 " %" PRIuMAX "\n",
	    nameorip,
	    protocol,
	    queue_size,
	    (uintmax_t)ctime);
  }
}

static int server_send_client_queues(struct packet_info_st *packetinfo){
  /*
   * This function sends the packetinfo to the nbs1 and nbs2 client
   * threads, and the emwin packetinfo to the emwin client threads.
   */
  struct emwin_queue_info_st emwinqueueinfo;
  int emwinqueueinfo_size = emwin_queue_info_get_size();
  size_t fpathout_size = EMWIN_FPATHOUT_SIZE + 1;
  size_t emwinfname_size = EMWIN_HEADER_FNAMESIZE + 1;
  int netclients;	
  int numentries;
  int emwinclients = 0;
  int protocol;
  int f_thread_created;
  char *nameorip;
  int status = 0;
  int status1 = 0;	/* error from emwin filter */
  int dberror = 0;
  int i;

  /*
   * Get total number of network clients, the total number
   * of entries and count the number of emwin clients.
   */
  netclients = conn_table_get_netclients(g.ct);   
  numentries = conn_table_get_numentries(g.ct);
  for(i = 0; i < numentries; ++i){
    if(conn_table_element_isnetclient_running(g.ct, i) == 0){
      continue;
    }
    protocol = get_client_protocol(g.ct, i);
    if(protocol == PROTOCOL_EMWIN)
      ++emwinclients;
  }
  
  /*
   * Call the emwinfilter to process the raw data
   * file and create the emwin file once and for all.
   */
  if((emwinclients > 0) ||
     (g.f_emwinserver_enabled && g.emwinfilter_always)){

    status1 = exec_emwin_filter(packetinfo,
				emwinqueueinfo.fpathout, fpathout_size,
				emwinqueueinfo.emwinfname, emwinfname_size);
    if(status1 == 0)
      log_verbose(1, "Sending %s to emwin queues.", emwinqueueinfo.fpathout); 
    else if(status1 == 1)
      log_verbose(2, "%s did not pass the emwin filter.", packetinfo->fpath); 
    else if(status1 == -1)
      log_errx("emwin filter error processing %s.", packetinfo->fpath);
  }

  if(netclients == 0){
    /* 
     * The table contains just the listening socket.
     */
    return(0);
  }

  /*
   * Execute the netfilter to set up the allow/deny distribution
   * policy for each client, for the current product.
   */
  if(gnetfilterp != NULL){
    status = exec_net_filter(gnetfilterp, g.ct);

    if(status != 0){
      log_err("Error processing the netfilter.");
      return(0);
    }
  }

  /*
   * We loop through the whole table.
   */
  numentries = conn_table_get_numentries(g.ct);
  for(i = 0; i < numentries; ++i){
    if(conn_table_element_isnetclient(g.ct, i) == 0)
      continue;

    protocol = get_client_protocol(g.ct, i);
    f_thread_created = conn_table_get_element_fthread_created(g.ct, i);
    nameorip = conn_element_get_nameorip(&g.ct->ce[i]);

    if(conn_table_element_isnetclient_running(g.ct, i) == 0){
      if(f_thread_created == 0){
	log_verbose(1, "Client thread %d [%s] not ready.", i, nameorip);
      } else if (conn_table_element_isnetclient_stoped(g.ct, i)){
	log_warnx("Client thread %d [%s] stoped.", i, nameorip);
      }
      continue;
    }

    if((g.netfilter_enable == 1) &&
       (allow_filter_get_flag(&g.ct->ce[i]) == 0)){
      log_verbose(1, "%s is not allowed for %s", packetinfo->fname,
		  conn_element_get_nameorip(&g.ct->ce[i]));
      continue;
    } else {
      log_verbose(1, "Queueing %s for client %d [%s]", packetinfo->fname,
		  i, conn_element_get_nameorip(&g.ct->ce[i]));
    }

    if((protocol == PROTOCOL_EMWIN) && (status1 == 0)){
      status = connqueue_snd(g.ct->ce[i].cq,
			     &emwinqueueinfo, emwinqueueinfo_size, &dberror);
    } else if((protocol == PROTOCOL_NBS1) || (protocol == PROTOCOL_NBS2)){
      status = connqueue_snd(g.ct->ce[i].cq,
			     packetinfo->packet, packetinfo->packet_size,
			     &dberror);
    }

    /*
     * The connqueue_snd() function ultimately calls the libqdb function
     * which, in the case of any of the errors below, also sets the
     * status flags in the queue structure. The client thread must
     * check (periodically) those flags and act accordingly. Here
     * it just reports the error.
     */
    if(status == -1)
      log_err2_db("Error writing to client queue", nameorip, dberror);
    else if(status == 1)
      log_info("Soft limit reached for client thread %d [%s].", i, nameorip);
    else if(status == 2)
      log_errx("Hard limit reached for client thread %d [%s].", i, nameorip);
  }

  return(0);
}

static int get_reload_server_filters_flag(void){

  int r = 0;

  if(pthread_mutex_trylock(&greload_server_filters_mutex) == 0){
    r = greload_server_filters;
    greload_server_filters = 0;
    pthread_mutex_unlock(&greload_server_filters_mutex);
  }else
    log_info("Cannot lock mutex in get_reload_server_filters_flag().");

  return(r);
}

void set_reload_server_filters_flag(void){

  int status = 0;

  if((status = pthread_mutex_lock(&greload_server_filters_mutex)) == 0){
    greload_server_filters = 1;
    pthread_mutex_unlock(&greload_server_filters_mutex);
  }else
    log_errx("Error %d locking mutex in set_reload_server_filters_flag().",
	     status);
}

static int get_server_state_flag(void){

  int r = 0;

  if(greport_client_connections == 0)
    return(0);

  if(pthread_mutex_trylock(&greport_client_connections_mutex) == 0){
    r = greport_client_connections;
    greport_client_connections = 0;
    pthread_mutex_unlock(&greport_client_connections_mutex);
  }else
    log_info("Cannot lock mutex in get_report_client_conn_flag().");

  return(r);
}

void set_server_state_flag(void){

  int status = 0;

  if((status = pthread_mutex_lock(&greport_client_connections_mutex)) == 0){
    greport_client_connections = 1;
    pthread_mutex_unlock(&greport_client_connections_mutex);
  }else
    log_errx("Error %d locking mutex in set_report_client_conn_flag().",
	     status);
}

#if 0
/*
 * See comment in the top of this file about these two functions.
 */
static int get_process_connections_flag(void){

  int r = 0;

  if(gprocess_connections == 0)
    return(0);

  if(pthread_mutex_trylock(&gprocess_connections_mutex) == 0){
    r = gprocess_connections;
    gprocess_connections = 0;
    pthread_mutex_unlock(&gprocess_connections_mutex);
  }else
    log_info("Cannot lock mutex in get_process_connections_flag().");

  return(r);
}

void set_process_connections_flag(void){

  int status = 0;

  if((status = pthread_mutex_lock(&gprocess_connections_mutex)) == 0){
    gprocess_connections = 1;
    pthread_mutex_unlock(&gprocess_connections_mutex);
  }else
    log_errx("Error %d locking mutex in set_process_connections_flag().",
	     status);
}
#endif

static int get_process_connections_flag(void){

  time_t now;

  now = time(NULL);
  if(now < gprocess_connections_time)
    return(0);

  gprocess_connections_time = now + SERVER_PROCESS_CONNECTIONS_PERIOD_SECS;

  return(1);
}

static int match_client_protocol(struct conn_table_st *ct, int i){
  /*
   * Returns 0 if the protocol is being served or 1 otherwise.
   */  
  int protocol;
  int status = 1;	/* does not match */

  assert(g.f_server_thread_created != 0);

  protocol = get_client_protocol(ct, i);
  if((g.servertype & protocol) != 0)
      status = 0;

  return(status);
}

/*
 * emwinfilter functions
 */
static int exec_emwin_filter(struct packet_info_st *packetinfo,
			     char *fpathout, size_t fpathout_size,
			     char *emwinfname, size_t emwinfname_size){
  /*
   * This function is called by the server to decide which files
   * to distribute to the emwin clients. The function uses the
   * emwin filter mechanism.
   *
   * Must return: 
   *   0 => send the file
   *   1 => do not send the file
   *  -1 => error; cannot tell
   */
  char *fname;
  int status = 0;
  int n;
  unsigned char b[4];
  int output_status;
  size_t output_fpathout_size = 0;
  size_t output_emwinfname_size = 0;

  fname = packetinfo->fname;
  status = emwin_regex_match(fname);
  if(status != 0)
    return(status);

  if(gemwinfilterp == NULL){
    /*
     * The e_emwin_filter_read() function closes the filter if it gets
     * an error. It is also closed here if there is a write error.
     */
    log_errx("Server filter is not opened. Will try to reopen.");
    if(open_emwin_filter() != 0){
      log_err("Cannot open emwin filter.");
      return(-1);
    }
  }

  n = pfilter_vprintf(gemwinfilterp->fp,
		      "%u %d %d %d %d %s %s\n", 
		      (unsigned int)packetinfo->seq_number,
		      packetinfo->psh_product_type,
		      packetinfo->psh_product_category,
		      packetinfo->psh_product_code,
		      packetinfo->np_channel_index,
		      packetinfo->fname,
		      packetinfo->fpath);

  if(n < 0)
    status = -1;

  if(status == 0)
    status = pfilter_flush(gemwinfilterp);

  if(status != 0){
    log_err("Could not write to emwin filter.");
    close_emwin_filter();

    return(-1);
  }

  /*
   * The output is of the form (see emwinfilter.tcl.in):
   *
   *
   *    <byte><2 bytes><2 bytes><fpathout><emwinfname>
   *
   * where 
   *    the first byte is the status (0 passed, 1 did not pass),
   *    the next two are 16 bit integers big endian order)
   *    giving the size of the two strings that follow.
   *
   * The strings are output only if the status is 0.
   */
  status = e_emwin_pfilter_read(gemwinfilterp, b, 1);
  if(status == 0){
    output_status = b[0];  
    if(output_status != 0)
      return(output_status);
  }else
    return(-1);

  status = e_emwin_pfilter_read(gemwinfilterp, b, 4);
  if(status == 0){
    output_fpathout_size = (b[0] << 8) + b[1];
    output_emwinfname_size = (b[2] << 8) + b[3];

    if(output_fpathout_size > fpathout_size - 1){
      close_emwin_filter();
      log_errx("Path too long: %d", output_fpathout_size);
      status = -1;
    } else {
      status = e_emwin_pfilter_read(gemwinfilterp, fpathout,
				  output_fpathout_size);
      fpathout[output_fpathout_size] = '\0';
    }
  }

  if(status == 0){
    if(output_emwinfname_size > emwinfname_size - 1){
      close_emwin_filter();
      log_errx("Emwin fname too long: %d", output_emwinfname_size);
      status = -1;
    } else {
      status = e_emwin_pfilter_read(gemwinfilterp, emwinfname,
				  output_emwinfname_size);
      emwinfname[output_emwinfname_size] = '\0';
    }
  }

  return(status);
}

static int e_emwin_pfilter_read(struct pfilter_st *filterp,
				void *buffer, size_t size){
  int status;

  status = pfilter_read(filterp, buffer, size);
  if(status == -1)
    log_err("Error reading from the emwin filter.");
  else if(status != 0){
    log_errx("Error reading from the emwin filter. Short count");
    status = -1;
  }

  if(status != 0){
    /*
     * Close the filter to force resynchronization.
     */
    close_emwin_filter();
  }

  return(status);
}

static int open_emwin_filter(void){

  assert(gemwinfilterp == NULL);

  gemwinfilterp = pfilter_open_wr(g.emwinfilter,
				  g.emwinfilter_fifo,
				  g.emwinfilter_read_timeout_secs);
  if(gemwinfilterp == NULL){
    return(-1);
  }

  return(0);
}

static void close_emwin_filter(void){

  if(g.f_emwinserver_enabled == 0)
    return;

  assert(gemwinfilterp != NULL);
  pfilter_vprintf(gemwinfilterp->fp, "\n");
  pfilter_flush(gemwinfilterp);
  pfilter_close(gemwinfilterp);
  gemwinfilterp = NULL;
}

/*
 * These are the functions to support the creation of the client
 * threads.
 */
static void spawn_client_threads(void){
  /*
   * Loop through the conn table and spawn the client thread for
   * each element whose protocol has been identified
   * and for which the thread has not been spawned yet. The
   * remaining initialization steps of each client are done here. The protocol
   * identification routine ends up calling conn_element_init2.
   * Here we setup the filter data, which calls init3, and then finally init4.
   */
  int numentries;
  int isnetclient;
  int f_thread_created;
  int protocol;
  struct connqueue_param_st cqparam;
  int status;
  int dberror = 0;
  char *nameorip;
  int i;

  /*
   * Initialize the cqparam with the parameters appropriate for nbs1, nbs2
   * clients.
   */
  cqparam.cache_mb = g.client_queue_dbcache_mb;
  cqparam.reclen = gpacketinfo.packet_size;
  cqparam.softlimit = g.client_queue_maxsize_soft;
  cqparam.hardlimit = g.client_queue_maxsize_hard;

  numentries = conn_table_get_numentries(g.ct);
  for(i = 0; i < numentries; ++i){
    f_thread_created = conn_table_get_element_fthread_created(g.ct, i);
    isnetclient = conn_element_isclient(&g.ct->ce[i]);
    protocol = get_client_protocol(g.ct, i);
    nameorip = conn_element_get_nameorip(&g.ct->ce[i]);

    if(f_thread_created || (isnetclient == 0) ||
       (protocol == PROTOCOL_UNKNOWN))
      continue;

    /*
     * A different cqparam.reclen for each protocol.
     */
    cqparam.reclen = gpacketinfo.packet_size;
    if(protocol == PROTOCOL_EMWIN){
      cqparam.reclen = emwin_queue_info_get_size();
    }
   
    /*
     * The write options has already been initialized by init1 to the
     * global value. Here we look to see if there is a per-client setting
     * for this client.
     */
    if(valid_str(g.clientoptions))
      lookup_client_options(&g.ct->ce[i], g.clientoptions);

    status = allow_filter_init(&g.ct->ce[i]);
    if(status == 0){
      status = conn_element_init4(&g.ct->ce[i],
				  client_thread_create,
				  client_thread_kill,
				  &cqparam,
				  &dberror);
    }
    if(status != 0){
      /*
       * The element must be removed from the table.
       */
      log_err2_db("Cannot spawn client thread", nameorip, dberror);
      (void)poll_kill_client_connection(g.ct, i);
      numentries = conn_table_get_numentries(g.ct);      
    } else {
      log_info("Spawned client thread for %s.", nameorip);
    }
  }
}

static void lookup_client_options(struct conn_element_st *ce,
				 char *clientoptions){
  /*
   * Looks in the per-host client option string to see if the client
   * appears, and then override those that were configured for this client.
   */
  char *nameorip;
  char *p;
  int val;
  int a[4];
  int count = 0;

  assert(clientoptions != NULL);

  nameorip = conn_element_get_nameorip(ce);
  p = strstr(clientoptions, nameorip);
  if(p == NULL)
    return;

  /*
   * Client found.
   */

  /*
   * The strategy is to initialize the a[] with the current (default) values
   * of the ce, then fill the a[] with the overrides, and finally copy the a[]
   * back to the ce.
   */ 
  a[0] = ce->write_timeout_ms;
  a[1] = ce->write_timeout_retry;
  a[2] = ce->reconnect_wait_sleep_secs;
  a[3] = ce->reconnect_wait_sleep_retry;

  while(p != NULL){
    /*
     * Find the separating character and point to the next parameter.
     */
    p = strchr(p, CLIENTOPTION_SEP_CHAR);
    if(p != NULL){
      ++p;
      if(sscanf(p, "%d", &val) == 1)
	a[count] = val;
      
      ++count;
    }
  }

  ce->write_timeout_ms = a[0];
  ce->write_timeout_retry = a[1];
  ce->reconnect_wait_sleep_secs = a[2];
  ce->reconnect_wait_sleep_retry = a[3];
}

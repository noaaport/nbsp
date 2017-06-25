/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "globals.h"
#include "file.h"
#include "err.h"
#include "signal.h"
#include "readn.h"
#include "packfpu.h"
#include "nbspq.h"
#include "util.h"
#include "nbspmspoolbdb.h"
#include "spooltype.h"
#include "filters.h"

#define INITIAL_SIZE		1
#define GROW_FACTOR		2

/*
 * The gflist is accesed only by the filter thread. But the greload_filters
 * flag is read by this module and set by main(), so it should be accessed only
 * by the set() and get() functions, which protect it with a mutex.
 *
 * The filter provides its own allocation for the packetinfo->packet
 * that is retrieved from the queue, to avoid having the qdb functions
 * to allocate memory for it for every packet.
 */
static struct filter_list_st gflist = {0, 0, NULL};
static struct packet_info_st gpacketinfo =
  {0, 0, 0, 0, 0, NULL, NULL, NULL, 0};
static int greload_filters = 0;	    
static pthread_mutex_t greload_filters_mutex = PTHREAD_MUTEX_INITIALIZER;
static int get_reload_filters_flag(void);

static int open_filter_server(void);
static void close_filter_server(void *arg);
static int create_filter_thread(void);
static void *filter_main(void *data);
static int filter_loop(void);
static int process_filter_queue(void);

static int init_unpack_packet_pool(void);
static void cleanup_unpack_packet_pool(void);

static int e_get_filters(void);
static int e_reload_filters(void);

static int init_filter_list(void);
static void kill_filter_list(void);
static int grow_list(void);
static int find_empty_entry(void);
static int find_entry(char *path);
static int process_dir_entry(char *dir, char *name);
static int add_filter_entry(char *fname, int type);
static void delete_filter_entry(int i);
static void sendto_filters(struct packet_info_st *packetinfo, int timeout_ms);
static int sendto_one_filter(int i, struct packet_info_st *packetinfo,
			     int timeout_ms);
static int sendto_fifo_filter(int i, struct packet_info_st *packetinfo,
			      int timeout_ms);
static int sendto_pipe_filter(int i, struct packet_info_st *packetinfo);
static int get_dev_filters(char *filterdir);
static int get_sys_filters(char *flist, int filter_type);

static void open_filters1(int reopen_flag);
static void open_filters(void);
static void reopen_filters(void);
static int open_one_filter(int i);
static int reopen_one_filter(int i);
static int open_pipe_filter(int i);
static int open_fifo_filter(int i);
static int close_pipe_filter(int i);
static int close_fifo_filter(int i);

/*
 * Support for the filter server state
 */
static int greport_filterserver_state = 0;
static pthread_mutex_t greport_filterserver_state_mutex = 
	PTHREAD_MUTEX_INITIALIZER;

static int get_filterserver_state_flag(void);
static void report_filters_stats(void);
static void reset_filters_stats(void);

static int open_filter_server(void){

  int status;

  status = init_unpack_packet_pool();
 
  if(status == 0)
    status = init_filter_list();

  if(status == 0){
    status = e_get_filters();
    open_filters();
  }

   return(status);
}

static void close_filter_server(void *arg __attribute__ ((unused))){

    kill_filter_list();
    cleanup_unpack_packet_pool();
}

void kill_filter_thread(void){
  /*
   * This "joins" the filter thread. 
   */

  if(g.f_filter_thread_created == 0)
    return;

  pthread_join(g.filter_thread_id, NULL);
  log_info("Finished filter server.");
}

int spawn_filter_server(void){

  int status = 0;

  /*
   * Because of the restrictions of tcl, the pfilter functions must
   * be called from only one thread. Therefore, the filter thread itself
   * must open and also close the filters. The best is then to let the
   * filter thread initialize everything. If something fails, it
   * signals the other threads by setting the quit flag.
   */

  status = create_filter_thread();

  return(status);
}

static int create_filter_thread(void){
  /*
   * This is the entry to the filter module. It is called by main after
   * forking, to spawn the filter server thread.
   */
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
    status = pthread_create(&t_id, &attr, filter_main, NULL);

  if(status != 0){
    log_err("Cannot create filter thread.");
  }else{
    g.filter_thread_id = t_id;
    g.f_filter_thread_created = 1;
    log_info("Spawned filter thread.");
  }

  return(status);
}

static void *filter_main(void *data __attribute__ ((unused))){

  int status;

  pthread_cleanup_push(close_filter_server, NULL);

  status = open_filter_server();
  if(status != 0){
    /*
     * See comment in processor_thread_main() in nbsp.c.
     */
    set_quit_flag();
  }

  while(get_quit_flag() == 0){
    status = filter_loop();
  }

  pthread_cleanup_pop(1);

  return(NULL);
}

static int filter_loop(void){

  int status = 0;

  while((status == 0) && (get_quit_flag() == 0)){
    status = process_filter_queue();

    if(status == 0){
      if(get_reload_filters_flag() == 1)
	status = e_reload_filters();
    }

    if(get_filterserver_state_flag()){
      report_filters_stats();
      reset_filters_stats();
    }
  }

  return(status);
}

static int process_filter_queue(void){

  int status = 0;
  size_t size;

  assert(gpacketinfo.packet != NULL);
  size = gpacketinfo.packet_size;
  status = e_nbspq_rcv_filter(&gpacketinfo.packet, &size);
  assert(size == gpacketinfo.packet_size);

  if(status == 1){
    /*
     * The queue was empty (and timedout waiting).
     */
    return(0);
  }else if(status != 0)
    return(-1);

  status = nbsfp_unpack_fpath(&gpacketinfo);

  if(status == 0)
    sendto_filters(&gpacketinfo, g.fifo_write_timeout_ms);
  else if(status == -1){
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

static int init_filter_list(void){

  int i;

  gflist.n = 0;
  gflist.nmax = 0;

  gflist.filters = malloc(INITIAL_SIZE * sizeof(struct filter_entry_st));

  if(gflist.filters == NULL){
    log_err("Cannot initialize filters module.");
    return(-1);
  }else
    gflist.nmax = INITIAL_SIZE;

  for(i = 0; i < gflist.nmax; ++i){
    gflist.filters[i].fd = -1;
    gflist.filters[i].pfp = NULL;
    gflist.filters[i].type = FILTER_TYPE_NONE;
    gflist.filters[i].fname = NULL;
    gflist.filters[i].stats.files_sent = 0;
    gflist.filters[i].stats.errors = 0;
  }

  return(0);
}

static void kill_filter_list(void){

  int i;

  if(gflist.filters == NULL)
    return;

  for(i = 0; i < gflist.nmax; ++i)
    delete_filter_entry(i);

  free(gflist.filters);

  gflist.n = 0;
  gflist.nmax = 0;
  gflist.filters = NULL;
}

static int init_unpack_packet_pool(void){

  if(nbsfp_packetinfo_init(&gpacketinfo) != 0){
    log_err("Cannot initalize the filter packetinfo memory pool.");
    return(-1);
  }

  return(0);
}

static void cleanup_unpack_packet_pool(void){

  nbsfp_packetinfo_cleanup(&gpacketinfo);
}

static int get_reload_filters_flag(void){

  int r = 0;

  if(pthread_mutex_trylock(&greload_filters_mutex) == 0){
    r = greload_filters;
    greload_filters = 0;
    pthread_mutex_unlock(&greload_filters_mutex);
  }else
    log_info("Cannot lock mutex in get_reload_filters_flag.");

  return(r);
}

void set_reload_filters_flag(void){

  int status = 0;

  if((status = pthread_mutex_lock(&greload_filters_mutex)) == 0){
    greload_filters = 1;
    pthread_mutex_unlock(&greload_filters_mutex);
  }else
    log_errx("Error %d locking mutex in set_reload_filters_flag.", status);
}

static void sendto_filters(struct packet_info_st *packetinfo, int timeout_ms){

  int status = 0;
  int i;

  if(gflist.n == 0)
    return;

  for(i = 0; i < gflist.nmax; ++i){
    status = sendto_one_filter(i, packetinfo, timeout_ms);
    if(status == 1){
      /*
       * This filter is not listening. Assume that it is a temporary
       * situation and let sendto_one_filter() continue trying
       * to reopen it.
       */
      log_warnx("Filter %s not listening", gflist.filters[i].fname);
      /*
       * log_info("Removing %s.", gflist.filters[i].fname);
       * delete_filter_entry(i);
       */
    } else if(status == -1){
      /*
       * Write error. Same as above.
       */
      log_err_write(gflist.filters[i].fname);
      /*
       * log_info("Removing %s.", gflist.filters[i].fname);
       * delete_filter_entry(i);
       */
    } else if(status != 0){
      /*
       * Write timeouts.
       * Only fifos can report these; assume it is a temporary situation.
       */
      log_errx("Timed out writing to %s.", gflist.filters[i].fname);
    }
  }
}

static int e_get_filters(void){

  int status = 0;

  if(valid_str(g.sysfilterlist)){
    status = get_sys_filters(g.sysfilterlist, FILTER_TYPE_PIPE);
    if(status != 0)
      log_err2("Could not load filters from list", g.sysfilterlist);
  }

  if(status == 0){
    if(valid_str(g.filterdevdir) && (dir_exists(g.filterdevdir) == 0)){
      status = get_dev_filters(g.filterdevdir);
      if(status != 0)
	log_err2("Could not load filters from", g.filterdevdir);
    }
  }

  if((status == 0) && (gflist.n > 0))
    log_info("Loaded filters.");

  return(status);
}

static int e_reload_filters(void){
  /*
   * First close all the pipes, so that those that were removed or replaced
   * in the dev directory are deleted or reloaded as the case may be. For
   * the fifos we keep them open because we close them when we get an error
   * from a reader closing the other end and so it is deleted automatically.
   */
  int status = 0;
  int i;

  for(i = 0; i < gflist.nmax; ++i){
    if(gflist.filters[i].type == FILTER_TYPE_PIPE)
	delete_filter_entry(i);
  }

  status = e_get_filters();
  reopen_filters();
  
  return(status);
}

static int sendto_one_filter(int i, struct packet_info_st *packetinfo,
			     int timeout_ms){

  int status = 0;

  assert((i >= 0) && (i < gflist.nmax));

  if(gflist.filters[i].fname == NULL)
    return(0);

  assert(gflist.filters[i].type != FILTER_TYPE_NONE);

  if(gflist.filters[i].type == FILTER_TYPE_FIFO)
    status = sendto_fifo_filter(i, packetinfo, timeout_ms);
  else if(gflist.filters[i].type == FILTER_TYPE_PIPE)
    status = sendto_pipe_filter(i, packetinfo);

  ++gflist.filters[i].stats.files_sent;
  if(status != 0)
    ++gflist.filters[i].stats.errors;

  return(status);
}

static int sendto_fifo_filter(int i, struct packet_info_st *packetinfo,
			      int timeout_ms){
  /*
   * If the filter is closed we open it and leave it open until
   * the other side closes it or we get an error.
   * 
   * Returns:
   *   0 => no errors
   *   1 => the filter is closed on the other side
   *  -1 => open or write error (other than filter closed)
   *  -2 => short read (timed out)
   */
  int status = 0;
  ssize_t n;

  if(gflist.filters[i].fd == -1){
    status = open_fifo_filter(i);
    if(status != 0)
      return(status);
  }

  n = writem(gflist.filters[i].fd,
	     packetinfo->packet, packetinfo->packet_size, timeout_ms, 0);

  if(n == -1){
    status = -1;
  }else if((size_t)n != packetinfo->packet_size){
    /*
     * timed out (including partial write)
     */
    status = -2;
  }

  if(status != 0){
    (void)close_fifo_filter(i);
  }

  return(status);
}

static int sendto_pipe_filter(int i, struct packet_info_st *packetinfo){
  /*
   * If the filter is closed we open it and leave it open until
   * the other side closes it.
   * To pipes we send the data formatted in one character string, with
   * each element separated by a blank. This is likely to be more useful
   * than sending the binary packet if the pipe is a script, which
   * probably is what most pipes will be.
   * 
   * Returns:
   *   0 => no errors
   *   1 => the filter is closed on the other side
   *  -1 => open or write error (other than filter closed)
   */
  int status = 0;
  int n;
  size_t fsize;

  if(gflist.filters[i].pfp == NULL){
    status = open_pipe_filter(i);
    if(status != 0)
      return(status);
  }

  /*
   * Check if we need the metadata and get it first.
   */
  if(spooltype_mspool()){
    if(nbsp_mspoolbdb_fpathsize(packetinfo->fpath, &fsize) != 0)
      fsize = 0;      /* This tells the filter that it was not available */
  }

  n = pfilter_vprintf(gflist.filters[i].pfp->fp,
		      "%u %d %d %d %d %s %s", 
		      (unsigned int)packetinfo->seq_number,
		      packetinfo->psh_product_type,
		      packetinfo->psh_product_category,
		      packetinfo->psh_product_code,
		      packetinfo->np_channel_index,
		      packetinfo->fname,
		      packetinfo->fpath);
  if(n < 0) 
    status = -1;
  
  if((spooltype_mspool()) && (status == 0)){
    /* Add the metadata */
    n = pfilter_vprintf(gflist.filters[i].pfp->fp, " %" PRIuMAX,
			(uintmax_t)fsize);
    if(n < 0) 
      status = -1;
  }

  if(status == 0){
    n = pfilter_vprintf(gflist.filters[i].pfp->fp, "\n");
    if(n < 0) 
      status = -1;
  }

  if(status == 0)
    status = pfilter_flush(gflist.filters[i].pfp);

  if(status != 0)
    (void)close_pipe_filter(i);

  return(status);
}

static int grow_list(void){
  /*
   * Returns 0 if there are no errors or -1 otherwise.
   */
  struct filter_entry_st *f;
  int old_nmax;
  int newsize;
  int i;

  old_nmax = gflist.nmax;
  newsize = (gflist.nmax * GROW_FACTOR)*sizeof(struct filter_entry_st);
  f = realloc(gflist.filters, newsize);
  if(f == NULL)
    return(-1);

  gflist.nmax *= GROW_FACTOR;
  gflist.filters = f;

  for(i = old_nmax; i < gflist.nmax; ++i){
    gflist.filters[i].fd = -1;
    gflist.filters[i].pfp = NULL;
    gflist.filters[i].type = FILTER_TYPE_NONE;
    gflist.filters[i].fname = NULL;
    gflist.filters[i].stats.files_sent = 0;
    gflist.filters[i].stats.errors = 0;
  }

  return(0);
}  

static int add_filter_entry(char *fname, int type){

  int status = 0;
  int n;
  int fname_size;
  int i;

  i = find_entry(fname);
  if(i >= 0){
    /*
     * It is already in the list.
     */
    return(0);
  }

  if(gflist.n == gflist.nmax){
    status = grow_list();
    n = gflist.n;
  }else
    n = find_empty_entry();

  if(status == 0){
    assert(n != -1);
    fname_size = strlen(fname) + 1;
    gflist.filters[n].fname = malloc(fname_size);
    if(gflist.filters[n].fname != NULL){
      strncpy(gflist.filters[n].fname, fname, fname_size);
      gflist.filters[n].type = type;
      gflist.filters[n].stats.files_sent = 0;
      gflist.filters[n].stats.errors = 0;
      ++gflist.n;
    }else{
      status = -1;
    }
  }

  return(status);
}

static int find_empty_entry(void){
  /*
   * This function finds the first empty slot.
   */
  int i;

  for(i = 0; i < gflist.nmax; ++i){
    if(gflist.filters[i].fname == NULL)
      return(i);
  }

  return(-1);
}

static int find_entry(char *path){
  /*
   * This function checks to see if the named file (path) is already
   * in the list.
   */
  int i;

  for(i = 0; i < gflist.nmax; ++i){
    if((gflist.filters[i].fname != NULL) &&
       (strcmp(gflist.filters[i].fname, path) == 0)){
       return(i);
    }
  }

  return(-1);
}

static void delete_filter_entry(int i){

  if(gflist.filters[i].fd != -1)
     close(gflist.filters[i].fd);

  if(gflist.filters[i].pfp != NULL){
    /*
     * There are occasions in which the pclose() hangs waiting for
     * the filter to finish. For some reason, the filter does not "see"
     * the eof and stays waiting in the "gets". It happens only when
     * the filters queue is large, and it may be because the server
     * is writing too fast to the filter and the eof gets lost.
     * For this reason, we request explicitly the filters to quit (and
     * ignore errors here).
     */
    pfilter_vprintf(gflist.filters[i].pfp->fp, "\n");
    pfilter_flush(gflist.filters[i].pfp);
    pfilter_close(gflist.filters[i].pfp);
  }

  if(gflist.filters[i].fname != NULL)
    free(gflist.filters[i].fname);

  gflist.filters[i].fd = -1;
  gflist.filters[i].pfp = NULL;
  gflist.filters[i].type = FILTER_TYPE_NONE;
  gflist.filters[i].fname = NULL;
  gflist.filters[i].stats.files_sent = 0;
  gflist.filters[i].stats.errors = 0;

  --gflist.n;
}

static int get_dev_filters(char *dir){
  /*
   * This function reads the directory and installs all the filters
   * (fifos and pipes) that it finds there.
   */
  DIR *dirp;
  struct dirent *direntryp;
  int status = 0;
  /* struct dirent dentry; */

  dirp = opendir(dir);
  if(dirp == NULL)
    return(-1);

  /*
  do {
    status = readdir_r(dirp, &dentry, &der);
    if((status == 0) && (der != NULL))
      status = process_dir_entry(dir, der->d_name);

  } while((status == 0) && (der != NULL));
  */

  do {
    direntryp = readdir(dirp);
    if(direntryp != NULL)
      status = process_dir_entry(dir, direntryp->d_name);

  } while((status == 0) && (direntryp != NULL));

  closedir(dirp);

  return(status);
}

static int process_dir_entry(char *dir, char *name){

  int status = 0;
  struct stat sb;
  char *path;
  int size;
  int n;

  size = strlen(dir) + strlen(name) + 2;	/* '/' and NULL */
  path = malloc(size);
  if(path == NULL)
    return(-1);

  n = snprintf(path, size, "%s/%s", dir, name);
  assert(n == size - 1);

  status = stat(path, &sb);
  if(status == 0){
    if(S_ISFIFO(sb.st_mode)){
      status = add_filter_entry(path, FILTER_TYPE_FIFO);
    }else if(S_ISREG(sb.st_mode) && ((sb.st_mode & S_IXUSR) != 0)){
      status = add_filter_entry(path, FILTER_TYPE_PIPE);
    }
  }else
    log_err2("Could not get info for", path);

  free(path);

  return(status);
}

static int get_sys_filters(char *flist, int filter_type){
  /*
   * Here the filter list should be a string of the form
   *
   * flist = "filter1:filter2 ..."
   *
   * Entries can be separated by ': \t'.
   * The strings can be set in the runtime configuration file. 
   * Setting it to "" in the conf file, disables the compiled-in
   * system (embedded tcl) or user (pipes) filters.
   *
   * Returns:
   *  0 => ok
   *  1 =>    
   * -1 => memory error.
   */
  int status = 0;
  char *flistcopy, *p, *q;
  int size;

  size = strlen(flist);
  flistcopy = malloc(size + 1);
  if(flistcopy == NULL){
    status = -1;
    goto end;
  }else
    strncpy(flistcopy, flist, size + 1);
      
  q = flistcopy;
  while((q != NULL) && (status == 0)){
    p = strsep(&q, FILTERLIST_SEP_CHARS);
    if(p[0] != '\0'){
      status = add_filter_entry(p, filter_type);
    }      
  }

 end:

  if(flistcopy != NULL)
    free(flistcopy);

  return(status);
}

/*
 * Functions to open the filters.
 */
static void open_filters1(int reopen_flag){
  /*
   * If the reopen_flag is set, then only those filters that are currently
   * closed are opened. This is used when reloading the filters since in
   * that case only the pipes are closed and reopened (the fifos remain
   * opened).
   */
  int status = 0;
  int i;

  if(gflist.n == 0)
    return;

  for(i = 0; i < gflist.nmax; ++i){
    if(reopen_flag == 0)
      status = open_one_filter(i);
    else
      status = reopen_one_filter(i);
    if(status == -1){
      /*
       * Open error.
       */
      log_err_open(gflist.filters[i].fname);
      log_info("Removing %s.", gflist.filters[i].fname);
      delete_filter_entry(i);
    }else if(status == 1){
      /*
       * This filter is not listening, remove it also.
       */
      log_info("Removing %s. Filter not listening", gflist.filters[i].fname);
      delete_filter_entry(i);
    }
  }
}

static void open_filters(void){

  open_filters1(0);
}

static void reopen_filters(void){
  /*
   * This function checks if the filter is opened or closed, and opens
   * only those that are currenty closed.
   */
  open_filters1(1);
}

static int open_one_filter(int i){

  int status = 0;

  assert((i >= 0) && (i < gflist.nmax));

  if(gflist.filters[i].fname == NULL)
    return(0);

  assert(gflist.filters[i].type != FILTER_TYPE_NONE);

  if(gflist.filters[i].type == FILTER_TYPE_FIFO)
    status = open_fifo_filter(i);
  else if(gflist.filters[i].type == FILTER_TYPE_PIPE)
    status = open_pipe_filter(i);

  return(status);
}

static int reopen_one_filter(int i){
  /*
   * Open only if it is currently closed. Used when reloading the filters.
   */
  int status = 0;

  assert((i >= 0) && (i < gflist.nmax));

  if(gflist.filters[i].fname == NULL)
    return(0);

  assert(gflist.filters[i].type != FILTER_TYPE_NONE);

  if((gflist.filters[i].type == FILTER_TYPE_FIFO) &&
     (gflist.filters[i].fd == -1))
    status = open_fifo_filter(i);
  else if((gflist.filters[i].type == FILTER_TYPE_PIPE) &&
	  (gflist.filters[i].pfp == NULL))
    status = open_pipe_filter(i);

  return(status);
}

static int open_fifo_filter(int i){
  /*
   * Returns:
   *   0 => no errors
   *   1 => the fifo is closed on the other end (not listening)
   *  -1 => open error
   */
  int fd;

  assert(gflist.filters[i].fd == -1);

  fd = open(gflist.filters[i].fname, O_WRONLY | O_NONBLOCK, 0);
  if(fd == -1){
    if((errno == ENOENT) || (errno == ENXIO)){
      /* The filter is not listening. */
      return(1);
    }else
      return(-1);
  }

  gflist.filters[i].fd = fd;

  return(0);
}

static int open_pipe_filter(int i){
  /*
   * Returns:
   *   0 => no errors
   *  -1 => open error
   */
  struct pfilter_st *pfp;

  assert(gflist.filters[i].pfp == NULL);

  pfp = pfilter_open(gflist.filters[i].fname);
  if(pfp == NULL)
    return(-1);

  gflist.filters[i].pfp = pfp;

  return(0);
}

static int close_fifo_filter(int i){
  /*
   * Returns:
   *   0 => no errors
   *  -1 => close error
   */
  int status = 0;

  assert(gflist.filters[i].fd != -1);

  status = close(gflist.filters[i].fd);
  gflist.filters[i].fd = -1;

  return(status);
}

static int close_pipe_filter(int i){
  /*
   * Returns:
   *   0 => no errors
   *  -1 => close error
   */
  int status = 0;

  assert(gflist.filters[i].pfp != NULL);

  pfilter_close(gflist.filters[i].pfp);
  gflist.filters[i].pfp = NULL;

  return(status);
}

/*
 * Functions to support the filter servers state.
 */
static int get_filterserver_state_flag(void){

  int r = 0;

  if(greport_filterserver_state == 0)
    return(0);

  if(pthread_mutex_trylock(&greport_filterserver_state_mutex) == 0){
    r = greport_filterserver_state;
    greport_filterserver_state = 0;
    pthread_mutex_unlock(&greport_filterserver_state_mutex);
  }else
    log_info("Cannot lock mutex in get_filterserver_state_flag().");

  return(r);
}

void set_filterserver_state_flag(void){

  int status = 0;

  if((status = pthread_mutex_lock(&greport_filterserver_state_mutex)) == 0){
    greport_filterserver_state = 1;
    pthread_mutex_unlock(&greport_filterserver_state_mutex);
  }else
    log_errx("Error %d locking mutex in set_filterserver_state_flag().",
	     status);
}

static void report_filters_stats(void){
  /*
   * Print the number of files processed.
   */
  int i;
  FILE *fp = NULL;
  char *fname;
  time_t now;

  fname = g.filterserver_statefile;
  if(valid_str(fname) == 0)
    return;

  fp = fopen(fname, "a");
  if(fp == NULL){
    log_err_open(fname);
    return;
  }

  /* Same format as the server active and connections file */
  now = time(NULL);
  fprintf(fp, "- %u\n", (unsigned int)now);
  for(i = 0; i < gflist.nmax; ++i){
    if(gflist.filters[i].fname != NULL){
      fprintf(fp, "  %s %u %u\n",
	      gflist.filters[i].fname,
	      gflist.filters[i].stats.files_sent,
	      gflist.filters[i].stats.errors);
    }
  }
  fclose(fp);
}

static void reset_filters_stats(void){

  int i;

  for(i = 0; i < gflist.nmax; ++i){
    gflist.filters[i].stats.files_sent = 0;
    gflist.filters[i].stats.errors = 0;
  }
}

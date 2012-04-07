/*
 * Copyright (c) 2002-2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <poll.h>
#ifndef INFTIM
  #define INFTIM -1
#endif
#include <errno.h>
#include "conn.h"		/* client table */
#include "ce.h"
#include "sock.h"		/* our higher level (unix) socket functions */
#include "tcpsock.h"		/* same for network sockets */
#include "poll.h"

/*
 * The public functions poll_xxx return 0 or an error code. If the error
 * originated in a db function, then ct->dberror will have the db error,
 * otherwise ct->dberror will be zero (and therefore the error originated
 * in a system call or function).
 */

/*
 * functions local to this file
 */
static int poll_add_connection(struct conn_table_st *ct, int sindex,
			       struct client_options_st *clientopts);
static int poll_add_connection_local(struct conn_table_st *ct, int sindex,
				     struct client_options_st *clientopts);
static int poll_add_connection_net(struct conn_table_st *ct, int sindex,
				     struct client_options_st *clientopts);
static int poll_restore_connection(struct conn_table_st *ct,
				   char *ip, int client_fd);
static int poll_del_connection(struct conn_table_st *ct, int client_index);
static int poll_client_hangup(struct conn_table_st *ct, int client_index);

int poll_loop(struct conn_table_st *ct, struct client_options_st *clientopts){
  /*
   * poll() blocks.
   */

  return(poll_loop_wait(ct, INFTIM, clientopts));
}

int poll_loop_nowait(struct conn_table_st *ct,
		     struct client_options_st *clientopts){
  /*
   * poll() does not block (zero timeout).
   */

  return(poll_loop_wait(ct, 0, clientopts));
}

int poll_loop_wait(struct conn_table_st *ct,
		   int timeout, struct client_options_st *clientopts){
  /*
   * This function returns 0 if there were no errors. Otherwise:
   * -1 if poll() returned an error.
   * -2 if a new connection was requested but could not be established.
   * a positive integer, returned by the clientproc() handler function,
   * indicating that a client request could not be handled.
   */
  int i;
  int status = 0;

  for(i = 0; i <= ct->n - 1; ++i){
    ct->pfd[i].events = POLLIN;
  }

  if((i = poll(ct->pfd, ct->n, timeout)) == -1)
    return(-1);
  else if(i == 0)
    return(0);

  /*
   * Loop through server first.
   */
  for(i = 0; i <= ct->n - 1; ++i){
    if((conn_element_isserver(&ct->ce[i]) == 0) ||
       ((ct->pfd[i].revents & POLLIN) == 0)){
      continue;
    }

    status = poll_add_connection(ct, i, clientopts); 
    if(status == -1){
      /*
       * A real error
       */
      return(-2);
    }else if(status == -2){
      /*
       * accept was interrupted (client hang up or something) we are done.
       */
      return(0);      /* no errors */
    }else if(status == -3){
      /*
       * access denied
       */
      return(0);	/* no error */
    }
  }

  /*
   * Loop through the other fd's. We manage here only hangups, which can be
   * signaled in several ways [see poll_client_hangup() below].
   * For real POLLIN's we call the function pointed to by ct->connproc.
   *
   * Note that if a new client was added above by poll_add_connection(),
   * then it will be included in the loop below. It is important that
   * the revents flag is cleared when the client element is
   * initialized so that it is not mistakenly processed.
   * (This is done in conn_table_add_element_un() in conn.c.)
   */
  assert(ct->connproc != NULL);

  for(i = 0; (i <= ct->n - 1) && (status == 0); ++i){
    if(conn_element_isserver(&ct->ce[i]))
      continue;

    if(ct->pfd[i].revents == 0)
      continue;

    if(poll_client_hangup(ct, i)){
      status = poll_del_connection(ct, i);
      assert(status == 0);
    }else if(ct->pfd[i].revents & POLLIN){
      status = ct->connproc(ct, i);
    }
  }
  return(status);
}

static int poll_add_connection(struct conn_table_st *ct, int sindex,
			       struct client_options_st *clientopts){
  int status = 0;

  if(ct->ce[sindex].type == CONN_TYPE_SERVER_LOCAL)
    status = poll_add_connection_local(ct, sindex, clientopts);
  else if(ct->ce[sindex].type == CONN_TYPE_SERVER_NET)
    status = poll_add_connection_net(ct, sindex, clientopts);

  return(status);
}

static int poll_add_connection_local(struct conn_table_st *ct, int sindex,
				     struct client_options_st *clientopts){
  /*
   * Here "sindex" is the index in the poll table array of one of
   * the servers entries.
   *
   * Returns:
   *	0 no errors
   *   -1 if there are errors, either because the connection cannot
   *      be accepted or because the client cannot be added to the table.
   */
  int server_fd;
  int client_fd = -1;
  pid_t client_pid;
  char *client_ip = NULL;
  char *client_name = NULL;
  int client_type = CONN_TYPE_CLIENT_LOCAL;
  int status = 0;

  server_fd = ct->pfd[sindex].fd;
  assert(server_fd != -1);
  assert(ct->ce[sindex].type == CONN_TYPE_SERVER_LOCAL);

  client_fd = server_accept_conn(server_fd, &client_pid, clientopts);
  if(client_fd >= 0)
    status = conn_table_add_element(ct, client_fd, client_type, client_pid,
				    client_ip, client_name,
				    clientopts->write_timeout_ms,
				    clientopts->write_timeout_retry,
				    clientopts->reconnect_wait_sleep_secs,
				    clientopts->reconnect_wait_sleep_retry,
				    clientopts->queue_read_timeout_ms);
  if(status != 0)
    close(client_fd);

  return(status);
}

static int poll_add_connection_net(struct conn_table_st *ct, int sindex,
				   struct client_options_st *clientopts){
  /*
   * Returns:
   * 0 no errors
   * -1 if there are errors, either because the conenction cannot
   *    be accepted or because the client cannot be added to the table.
   * -2 if accept could not be completed (client hangup, interrupted, from
   *    server_accept_nconn()).
   * -3 if access is denied in hosts.allow.
   */
  int server_fd;
  int client_fd = -1;
  pid_t client_pid = -1;	/* netwok clients have pid = -1 */
  char *client_ip = NULL;
  char *client_name = NULL;
  int client_type = CONN_TYPE_CLIENT_NET;
  int status = 0;

  assert(ct->ce[sindex].type == CONN_TYPE_SERVER_NET);

  server_fd = ct->pfd[sindex].fd;
  assert(server_fd != -1);

  client_fd = tcp_server_accept_conn(server_fd, clientopts, NULL, NULL);
  if(client_fd < 0)
    return(client_fd);

  client_ip = get_peer_ip(client_fd);
  if(client_ip == NULL){
    close(client_fd);
    return(-1);
  }

  status = poll_restore_connection(ct, client_ip, client_fd);
  if(status == 0){
    /*
     * A client has reconnected and the (previously closed) connection
     * has been restored.
     */
    return(0);
  }else if(status == 1){
    /*
     * An error trying to restore a connection.
     */
    free(client_ip);
    close(client_fd);
    return(-3);
  }else{
    /*
     * When the status == -1 the new connection must be added.
     */
    status = 0;
  }

  /* 
   * If the name cannot be determined we just leave it null and continue.
   * The application will decide (tcpwrappers, etc) what it will do in
   * such cases.
   */
  client_name = get_peer_name(client_fd);

  /* tcpwrappers */
  if(ct->accessproc != NULL){
    if(ct->accessproc(client_fd, client_ip, client_name) == 0){
      /*
       * access denied in hosts.allow
       */
      if(client_name != NULL)
	free(client_name);

      free(client_ip);
      close(client_fd);
      return(-3);
    }
  }

  /*
   * If the maximum number of allowed connections is reached, set fd to
   * the same value returned when hosts.allow dennies a connection (-3).
   * The limit is disabled if it was initialized to a negative value.
   */
  if((ct->nclientsmax >= 0) && (ct->nclients == ct->nclientsmax)){
    close(client_fd);
    return(-3);
  }

  status = conn_table_add_element(ct, client_fd, client_type, client_pid,
				  client_ip, client_name,
				  clientopts->write_timeout_ms,
				  clientopts->write_timeout_retry,
				  clientopts->reconnect_wait_sleep_secs,
				  clientopts->reconnect_wait_sleep_retry,
				  clientopts->queue_read_timeout_ms);
  if(status != 0)
    close(client_fd);
  
  return(status);
}

static int poll_del_connection(struct conn_table_st *ct, int client_index){
  /*
   * Returns:
   *   0 if there are no errors.
   */
  int status = 0;

  status = conn_table_del_element(ct, client_index);

  return(status);
}

static int poll_client_hangup(struct conn_table_st *ct, int client_index){
  /*
   * Stevens unpv1 , p. 169-175.
   */
  struct pollfd *pfd;
  int hangup = 0;
  int n;
  char c;

  pfd = &ct->pfd[client_index];

  if(pfd->revents & (POLLHUP | POLLERR | POLLNVAL))
    hangup = 1;
  else if(pfd->revents & POLLIN){
    n = recvfrom(pfd->fd, &c, 1, MSG_PEEK, NULL, 0);
    if(n == 0){
      /*
       * Connection closed.
       */
      hangup = 2;
    }else if(n < 0){
      /*
       * Connection reset by peer (if errno == ECONNRESET).
       */
      hangup = -1;
    }
  }

  if((hangup != 0) && (ct->hangupproc != NULL))
    ct->hangupproc(ct, client_index, hangup);

  return(hangup);
}

int poll_kill_client_connection(struct conn_table_st *ct, int client_index){
  /*
   * This function can be used if for some reason the server wants
   * to disconect a particular client. I introduced the function
   * for the ByteBlaster server (npemwind).
   */

  return(poll_del_connection(ct, client_index));
}

static int poll_restore_connection(struct conn_table_st *ct,
				   char *ip, int client_fd){
  /*
   * This function searches the table to see if the client already exists.
   * The function returns:
   *  0 if the client was found, with a closed connection, and the connection
   *    could be restored.
   *  1 If the client exists as above, but the connection could not
   *     be restored or if the client exists with an active connection
   *     or unknown status, and the old connection could not be deleted.
   *     (The caller should remove the client from the table).
   * -1 If the client did not exist (the caller should then add it).
   * -1 If the client existed but it is trying to open a new connection,
   *    in which case this function deletes the old connection
   *    (the caller should then add the new connection).
   */
  int connection_status = 0;
  int i;

  i = conn_table_find_element_byip(ct, ip);
  if(i == -1)
    return(-1);

  connection_status = conn_element_get_connection_status(&ct->ce[i], NULL);
  if(connection_status == -1){
    /*
     * The client exists in the table but we could not determine its status.
     * Treat as if the connection was valid, like below.
     */
    if(poll_del_connection(ct, i) == 0)
      return(-1);
    else
      return(1);
  }else if(connection_status == 0){
    /*
     * The client exists in the table with an active and valid connection.
     * Since no duplicates are allowed, delete the previous and add a new one.
     */
    if(poll_del_connection(ct, i) == 0)
      return(-1);
    else
      return(1);
  }

  /*
   * connection_status == 1 => the client is in the table with a closed
   * connection and is trying to restore it.
   */
  if(conn_element_set_connection_status(&ct->ce[i], client_fd) != 0)
    return(1);
  
  ct->pfd[i].fd = client_fd;

  return(0);
}

/*
 * Copyright (c) 2002-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include "config.h"

#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "sock.h"

#ifndef HAVE_SUN_LEN
/* actual length of an initialized sockaddr_un */
#define SUN_LEN(su) \
        (sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))
#endif

static int set_socket_grm(char *name, char *group);

static int set_socket_grm(char *name, char *group){
  /*
   * Set the mode and group of the socket file. The mode
   * is rw for the group and owner. The group is set to "group"
   * and the owner is left unchanged.
   * Returns -1 on error, or 0 otherwise.
   */
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
  int status = 0;

  status = chgrpmode(name, group, mode);

  return(status);
}

int chgrpmode(char *name, char *group, mode_t mode){
  /*
   * Set the mode and group of a file; the owner is left unchanged.
   * Returns -1 on error, or 0 otherwise.
   * (For owner would use: p = getpwnam(name) and p->pw_uid.)
   */
  struct group *gr;
  int status = 0;

  if(group != NULL){
    gr = getgrnam(group);
    if(gr == NULL)
      return(-1);

    status = chown(name, -1, gr->gr_gid);
  }

  if(status == 0)
    status = chmod(name, mode);

  return(status);
}

int server_open_conn(char *name, char *group, int backlog){
  /*
   * This function creates the server endpoint of the connection.
   * Returns a file descriptor or -1 if socket(), bind() or
   * listen() fails, or -2 if the group/mode could not be set.
   */
  int status = 0;
  int fd = -1;
  int len;
  struct sockaddr_un saddr;

  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if(fd == -1)
    return(-1);
  
  /*
   * In case it exists
   */
  unlink(name);

  memset(&saddr, 0, sizeof(struct sockaddr_un));
  saddr.sun_family = AF_UNIX;
  if(strlen(name) >= sizeof(saddr.sun_path)){
    close(fd);
    errno = ENAMETOOLONG;
    return(-1);
  }
  strncpy(saddr.sun_path, name, strlen(name) + 1);

  /*
   * len = sizeof(saddr.sun_len) + sizeof(saddr.sun_family) + strlen(name) + 1;
   *
   * SUN_LEN gives the actual length of an initialized sockaddr_un
   * (excluding the terminating null), while the definition of
   * sun_len includes it. u_char sun_len (sockaddr len including null)
   */
  len = SUN_LEN(&saddr);

  status = bind(fd, (struct sockaddr*)&saddr, len);
  if(status == 0)
    status = listen(fd, backlog);

  if(status == 0){
    if(set_socket_grm(name, group) != 0)
      status = -2;
  }

  if(status != 0){
    close(fd);
    fd = status;
  }

  return(fd);
}

int server_accept_conn(int fd, pid_t *pid,
		       struct client_options_st *clientopts){
  /*
   * Returns the new fd for this connection if no errors or -1 otherwise.
   */
  int nfd;
  socklen_t len;
  struct sockaddr_un saddr;
  char *start, *end;

  len = sizeof(struct sockaddr_un);
  memset(&saddr, 0, sizeof(struct sockaddr_un));
  nfd = accept(fd, (struct sockaddr*)&saddr, &len);
  if(nfd == -1)
    return(-1);

  /*
   * Extract from the path name the id for this client.
   * We will use the pid, assumed to be contained in the last 5
   * characters of the file name.
   */

  len = strlen(saddr.sun_path);
  start = &(saddr.sun_path[len - 5]);
  *pid = (pid_t)strtol(start, &end, 10);
  if((end == start) || (*end != '\0')){
    /*
     * Not the expected format. 
     */
    close(nfd);
    nfd = -1; 

    /* In the real application we will return
     * an error, but in the debug version we will abort, since this is
     * either a (client) programing error or some kind of sabotage.
     */
    assert(nfd != -1);

    return(-1);
  }

  /*
   * This deletes the name, but the programs that have the socket open
   * can continue to use it.
   */
  (void)unlink(saddr.sun_path);

  if(clientopts->cloexec == 1){
    if(fcntl(nfd, F_SETFD, FD_CLOEXEC) == -1){
      close(nfd);
      return(-1);
    }
  }

  return(nfd);
}

int client_open_conn(char *server_name, char *client_basename){
  /*
   * This is the function that a client must call to connect to
   * the server.
   * Returns the fd for the client, or -1 if could not create the client socket
   * or -2 if could not connect to the server.
   */
  int fd;
  int len;
  pid_t pid = getpid();
  struct sockaddr_un server;
  struct sockaddr_un client;
  int mode = S_IRUSR | S_IWUSR;
  int status = 0;

  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if(fd == -1){
    return(-1);
  }

  memset(&client, 0, sizeof(struct sockaddr_un));
  client.sun_family = AF_UNIX;
#ifdef PID_T_INT
  snprintf(client.sun_path, sizeof(client.sun_path),
	   "%s%05d", client_basename, pid);
#else
  snprintf(client.sun_path, sizeof(client.sun_path),
	   "%s%05d", client_basename, (int)pid);
#endif
  len = SUN_LEN(&client);
  /*
   * In case it exists
   */
  unlink(client.sun_path);

  status = bind(fd,(struct sockaddr*)&client, len);
  if(status == 0)
    status = chmod(client.sun_path, mode);

  if(status != 0){
    close(fd);
    unlink(client.sun_path);
    return(-1);
  }

  /*
   * Connect to the server
   */
  memset(&server, 0, sizeof(struct sockaddr_un));
  server.sun_family = AF_UNIX;
  strncpy(server.sun_path, server_name, sizeof(server.sun_path) - 1);
  len = SUN_LEN(&server);
  if(connect(fd,(struct sockaddr*)&server, len) == -1){
    unlink(client.sun_path);
    close(fd);    
    return(-2);
  }

  return(fd);
}

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include "../libconn.h"
#include "common.h"

static int init_conn_table(int *fd, int numfd);
static int free_conn_table(void);
static int process_connection(struct conn_table_st *ct, int i);

static struct conn_table_st *gconnt = NULL;

static int process_connection(struct conn_table_st *ct, int i){

  int status = 0;
  int n;
  char buf[BUFSIZ];
  int fd;
  pid_t pid;
  char *addr;

  fd = conn_table_get_element_fd(ct, i);
  pid = conn_table_get_element_pid(ct, i);
  if(pid == -1){
    addr = conn_table_get_element_ip(ct, i);
    fprintf(stdout, "Connection from %s\n", addr);
  }else
    fprintf(stdout, "Connection from %d\n", pid);

  if((n = read(fd, buf, BUFSIZ)) > 0){
    buf[n] = '\0';
    fprintf(stdout, "%s\n", buf);
    strcpy(buf, "server: Received\n");
    write(fd, buf, strlen(buf));
  }else{
    perror("read");
    status = i;
  }

  return(status);
}


int main(void){

  int fd[2];
  int status = 0;

  /*
   * open the unix connection socket
   */
  fd[0] = server_open_conn(SERVER_SOCKET, "dialer", 5);
  if(fd[0] <= 0)
    err(1, "server_open_conn()");

  /*
   * the network socket
   */
  fd[1] = server_open_nconn(WMON_PORT, 5);
  if(fd[1] == -1)
    err(1, "server_open_nconn()");

  status = init_conn_table(fd, 2);

  while(status == 0){
    status = poll_loop(gconnt);
    fprintf(stdout, "status from poll loop: %d\n", status);
  }

  free_conn_table();
  close(fd[0]);
  close(fd[1]);
  unlink(SERVER_SOCKET);

  return(0);
}

static int init_conn_table(int *fd, int numfd){

  int status = 0;
  int i;

  gconnt = conn_table_create(2, process_connection, NULL);
  if(gconnt == NULL)
    err(1, "conn_table_create");

  status = conn_table_add_element(gconnt, fd[0], CONN_TYPE_SERVER_LOCAL,
				  0, NULL, NULL);
  if(status == 0)
    status = conn_table_add_element(gconnt, fd[1], CONN_TYPE_SERVER_NET,
				  0, NULL, NULL);

  if(status == 0){
    for(i = 2; (i <= numfd - 1) && (status == 0); +i)
      status = conn_table_add_element(gconnt, fd[i], CONN_TYPE_APPLICATION,
				  0, NULL, NULL);
  }

  if(status != 0)
    err(1, "conn_table_add_element");

  return(status);
}

static int free_conn_table(void){

  conn_table_destroy(gconnt);
  gconnt = NULL;

  return(0);
}

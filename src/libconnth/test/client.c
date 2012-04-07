/*
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "tcpsock.h"
#include "common.h"

int main(void){

  int sfd;
  int n;
  char buf[BUFSIZ];

  sfd = client_open_conn(SERVER_SOCKET, CLIENT_SOCKET, 5);
  if(sfd == -1)
    exit(1);

  while(fgets(buf, BUFSIZ, stdin) != NULL){
    n = strlen(buf);
    write(sfd, buf, n);
    n = read(sfd, buf, BUFSIZ);
    write(1, buf, n);
  }

  close(sfd);
  exit(0);
}




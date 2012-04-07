/*
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
*/
#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include "../tcpsock.h"
#include "common.h"

#define SERVER	"caribe"

int main(void){

  int sfd;
  int n;
  char buf;
  int gai_code;

  sfd = tcp_client_open_conn(SERVER, NBSP_PORT, &gai_code);
  if(sfd == -1){
    if(gai_code != 0)
      errx(1, "tcp_client. %s", gai_strerror(gai_code));
    else
      err(1, "tcp_client");
  }

  /*
  while(fgets(buf, BUFSIZ, stdin) != NULL){
    n = strlen(buf);
    write(sfd, buf, n);
    n = read(sfd, buf, BUFSIZ);
    write(1, buf, n);
  }
  */

  while((n = read(sfd, &buf, 1)) > 0){
    write(1, &buf, 1);
  }

  close(sfd);
  exit(0);
}

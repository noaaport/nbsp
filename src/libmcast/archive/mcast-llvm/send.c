#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <err.h>
#include <sys/utsname.h>
#include "mcast.h"

/*
#define SAP_NAME	"sap.mcast.net"
#define SAP_PORT	"9875"
*/
#define SAP_NAME	"224.0.1.1"
#define SAP_PORT	"1201"

static int loop(int s, struct sockaddr *sa, socklen_t sa_len);

int main(int argc, char **argv){

  int status = 0;
  int sfd = -1;
  socklen_t sa_len;
  /* struct sockaddr *sa; */
  void *sa;
  int gai_code;

  if(argc == 3)
    sfd = mcast_snd_default(argv[1], argv[2], 255, &sa, &sa_len,
			    &gai_code);
  else
    sfd = mcast_snd_default(SAP_NAME, SAP_PORT, 255, &sa, &sa_len,
			    &gai_code);

  if(sfd < 0){
    if(gai_code == 0)
      err(1, "mcast()");
    else
      errx(1, "mcast(): %s", gai_strerror(gai_code));
  }

  while(status == 0)
    status = loop(sfd, (struct sockaddr*)sa, sa_len);    /* send */

  return(status);

}

static int loop(int s, struct sockaddr *sa, socklen_t sa_len){
#define MAXLINE 1024

  char buf[MAXLINE + 1];
  ssize_t n;
  int status = 0;
  struct utsname name;

  if(uname(&name) != 0)
    err(1, "uname()");
  
  snprintf(buf, MAXLINE + 1, "%s: %d\n", name.nodename, getpid());
  fprintf(stdout, "%s", buf);
  n = sendto(s, buf, MAXLINE, 0, sa, sa_len);
  if(n == -1)
    err(1, "sendto");

  sleep(5);

  return(status);
}




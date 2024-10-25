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
#include <time.h>
#include "mcast.h"

/*
#define SAP_NAME	"sap.mcast.net"
#define SAP_PORT	"9875"
*/
#define SAP_NAME	"224.0.1.1"
#define SAP_PORT	"1201"

static int loop(int s, socklen_t sa_len);

int main(int argc, char **argv){

  int status = 0;
  int sfd = -1;
  socklen_t sa_len;
  struct sockaddr *sa;

  if(argc == 3)
    sfd = mcast_recv(argv[1], argv[2], (void **) &sa, &sa_len);
  else
    sfd = mcast_recv(SAP_NAME, SAP_PORT, (void **) &sa, &sa_len);

  if(sfd < 0)
    err(1, "mcast()");

  while(status == 0)
    status = loop(sfd, sa_len);    /* receive and print */

  return(status);

}


static int loop(int s, socklen_t sa_len){
#define MAXLINE 5200

  static struct sockaddr *sa = NULL;
  char buf[MAXLINE];
  socklen_t len;
  ssize_t n;
  int status = 0;
  static unsigned int total_data = 0;
  static unsigned int total_frames = 0;
  static time_t last = 0;
  time_t now;

  if(sa == NULL)
    sa = malloc(sa_len);

  if(sa == NULL)
    err(1, "malloc");

  if(last == 0)
    last = time(NULL);

  len = sa_len;
  n = recvfrom(s, buf, MAXLINE, 0, sa, &len);
  if(n == -1)
    err(1, "recvfrom");

  total_data += n;
  ++total_frames;
  now = time(NULL);
  if(now - last >= 5){
    n = total_data/(5 * 1024);
    fprintf(stdout, "%u\t%u\n", n, total_frames);
    last = now;
    total_data = 0;
    total_frames = 0;
  }

  /*
  fprintf(stdout, "From %s\n %d\n", sock_ntop(sa), n);
  */
  
  return(status);
}




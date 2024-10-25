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
#include "mcast.h"

/*
#define SAP_NAME	"sap.mcast.net"
#define SAP_PORT	"9875"
*/
#define SAP_NAME	"224.0.1.1"
#define SAP_PORT	"1201"

static char *ifname = NULL;
static char *ifip = NULL;

static int loop(int s, socklen_t sa_len);

int main(int argc, char **argv){

  int status = 0;
  int sfd = -1;
  socklen_t sa_len;
  struct sockaddr *sa;

  char *optstr = "i:p:";
  char *usage = "recv [-i ifname] [-p ip] <m_ip> <m_port>";
  int c;

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'i':
      ifname = optarg;
      break;
    case 'p':
      ifip = optarg;
      break;
    default:
      status = 1;
      errx(1, usage);
      break;
    }
  }

  argc -= optind;
  argv += optind;
  if(argc == 2)
    sfd = mcast_rcv(argv[0], argv[1], ifname, ifip, -1,
		    (void **) &sa, &sa_len);
  else
    sfd = mcast_rcv(SAP_NAME, SAP_PORT, ifname, ifip, -1,
		    (void **) &sa, &sa_len);

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

  if(sa == NULL)
    sa = malloc(sa_len);

  if(sa == NULL)
    err(1, "malloc");

  len = sa_len;
  n = recvfrom(s, buf, MAXLINE, 0, sa, &len);
  if(n == -1)
    err(1, "recvfrom");

  /*  fprintf(stdout, "From %s\n %d\n", sock_ntop(sa), n); */
  fprintf(stdout, "%d\n", n);

  return(status);
}




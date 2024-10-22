#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <err.h>
#include "mcast.h"

/*
#define SAP_NAME	"sap.mcast.net"
#define SAP_PORT	"9875"
*/

#define SAP_NAME	"224.0.1.1"
#define SAP_PORT	"1201"

#define FRAMESIZE	5200

static char *ifname = NULL;
static char *ifip = NULL;

static int loop(int s, socklen_t sa_len);
static int save_frame(char *buf, int frame_size);

int main(int argc, char **argv){

  int status = 0;
  int sfd = -1;
  socklen_t sa_len;
  /* struct sockaddr *sa; */
  void *sa;
  char *optstr = "i:I:";
  char *usage = "recv [-i ifname] [-I ip] <m_ip> <m_port>";
  int c;
  int gai_code;

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 'i':
      ifname = optarg;
      break;
    case 'I':
      ifip = optarg;
      break;
    default:
      status = 1;
      errx(1, "%s", usage);
      break;
    }
  }

  argc -= optind;
  argv += optind;
  if(argc == 2)
    sfd = mcast_rcv(argv[0], argv[1], ifname, ifip, -1,
		    &sa, &sa_len, &gai_code);
  else
    sfd = mcast_rcv(SAP_NAME, SAP_PORT, ifname, ifip, -1,
		    &sa, &sa_len, &gai_code);

  if(sfd < 0){
    if(gai_code == 0)
      err(1, "%s", "Error from mcast_rcv");
    else
      errx(1, "%s: %s", "Error from mcast_rcv", gai_strerror(gai_code));
  }

  while(status == 0)
    status = loop(sfd, sa_len);    /* receive and print */

  return(status);

}

static int loop(int s, socklen_t sa_len){

  static struct sockaddr *sa = NULL;
  char buf[FRAMESIZE];
  socklen_t len;
  ssize_t n;
  int status = 0;

  if(sa == NULL)
    sa = malloc(sa_len);

  if(sa == NULL)
    err(1, "%s", "Error from malloc");

  len = sa_len;
  n = recvfrom(s, buf, FRAMESIZE, 0, sa, &len);
  if(n == -1)
    err(1, "%s", "Error from recvfrom()");

  fprintf(stdout, "From %s\n %d\n", sock_ntop(sa), (int)n);
  save_frame(buf, n);
  
  return(status);
}

static int save_frame(char *frame, int frame_size){

  int fd;
  char fname[12];
  ssize_t n;
  static int i = 0;	/* increased in every loop (frame index) */

  snprintf(fname, 12, "%s.%d", "output", i);
  ++i;

  fd = open(fname, O_WRONLY | O_CREAT, 0666);
  if(fd == -1)
    err(1, "%s: %s", "Error opening", fname);

  n = write(fd, frame, (size_t)frame_size);
  if(n < frame_size)
    err(1, "%s", "Error from write()");

  close(fd);
	
  return(0);
  
}

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <err.h>
#include "mcast.h"

#define FAZZT_NAME	"239.255.0.9"
#define FAZZT_PORT	"8009"

static char *ifname = NULL;
static char *ifip = NULL;

/* static char *fazztdir = "/var/fazzt"; */
static char *fazztdir = "fazzt";
static char *fpath = NULL;
static int fpath_size = 0;

#define INT_MAX_SIZE 12		/* number of digits in MAX_INT */

static int loop(int s, socklen_t sa_len);
static void init_fpath(void);
static void save_packet(char *data, int data_size);

int main(int argc, char **argv){

  int status = 0;
  int sfd = -1;
  socklen_t sa_len;
  /* struct sockaddr *sa; */
  void *sa;
  char *optstr = "i:p:";
  char *usage = "recv [-i ifname] [-p ip] <m_ip> <m_port>";
  int c;
  int gai_code;

  init_fpath();

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
    sfd = mcast_rcv(FAZZT_NAME, FAZZT_PORT, ifname, ifip, -1,
		    &sa, &sa_len, &gai_code);

  if(sfd < 0){
    if(gai_code == 0)
      err(1, "mcast()");
    else
      errx(1, "mcast(): %s", gai_strerror(gai_code));
  }

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

  fprintf(stdout, "From %s\n %d\n", sock_ntop(sa), (int)n);
  save_packet(buf, n);

  return(status);
}

static void init_fpath(void){

  int size;

  /* "/dir/fname" */
  size = strlen(fazztdir) + 1 + INT_MAX_SIZE + 1;
  fpath = malloc(size);
  if(fpath == NULL)
    err(1, "malloc");

  fpath_size = size;
}

static void save_packet(char *data, int data_size){

  int fd;
  int n;
  static int count = 0;

  ++count;
  n = snprintf(fpath, fpath_size, "%s/%012d", fazztdir, count);
  assert(n == fpath_size - 1);
  fd = open(fpath, O_CREAT | O_WRONLY, 0644);
  if(fd == -1)
    err(1, "open");
  if(write(fd, data, data_size) == -1)
    err(1, "write");

  close(fd);
}


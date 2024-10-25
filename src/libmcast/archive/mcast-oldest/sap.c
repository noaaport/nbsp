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
#define SAP_NAME	"224.0.1.5"
#define SAP_PORT	"1205"

int udp_client(char *host, char *service, void **sa_ptr, socklen_t *sa_len);
int loop(int s, socklen_t sa_len);
const char *sock_ntop(struct sockaddr *sa);

int main(int argc, char **argv){

  int status = 0;
  int sfd = -1;
  int on = 1;
  socklen_t sa_len;
  struct sockaddr *sa;
  struct ip_mreq mreq;

  sfd = udp_client(SAP_NAME, SAP_PORT, (void **) &sa, &sa_len);
  if(sfd < 0)
    err(1, "udp_client");

  status = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  if(status == 0)
    status = bind(sfd, sa, sa_len);

  if(status == 0){
    memcpy(&mreq.imr_multiaddr, &((struct sockaddr_in*)sa)->sin_addr, 
	   sizeof(struct in_addr));

    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    status = setsockopt(sfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			&mreq, sizeof(struct ip_mreq));
  }

  if(status == -1){
    close(sfd);
    sfd = -1;
    err(1, "setsockopt");
  }

  while(status == 0)
    status = loop(sfd, sa_len);    /* receive and print */

  return(status);

}

int udp_client(char *host, char *service, void **sa_ptr, socklen_t *sa_len){

  int status = 0;
  int sfd = -1;
  struct addrinfo hints, *res, *res0;
  
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  status = getaddrinfo(host, service, &hints, &res0);
  if(status != 0)
    errx(1, "udp_client error for %s, %s: %s", host, service, 
	 gai_strerror(status));

  res = res0;
  while(res != NULL){
    sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sfd >= 0)
      break;          /* success */
    res = res->ai_next;
  }
  
  if (res == NULL)        /* errno set from final socket() */
    err(1, "udp_client error for %s, %s", host, service);

  *sa_ptr = malloc(res->ai_addrlen);
  if(*sa_ptr != NULL){
    memcpy(*sa_ptr, res->ai_addr, res->ai_addrlen);
    *sa_len = res->ai_addrlen;
  }else{
    close(sfd);
    sfd = -1;
    err(1, "malloc()");
  }

  freeaddrinfo(res0);

  return(sfd);
}

int loop(int s, socklen_t sa_len){
#define MAXLINE 1024

  static struct sockaddr *sa = NULL;
  char buf[MAXLINE + 1];
  socklen_t len;
  ssize_t n;
  int status = 0;
  struct sap_packet {
    uint32_t      sap_header;
    uint32_t      sap_src;
    char          sap_data[1];
  } *sapp;

  if(sa == NULL)
    sa = malloc(sa_len);

  if(sa == NULL)
    err(1, "malloc");

  len = sa_len;
  n = recvfrom(s, buf, MAXLINE, 0, sa, &len);
  if(n == -1)
    err(1, "recvfrom");

  buf[n] = 0;                     /* null terminate */
    
  sapp = (struct sap_packet *) buf;
  if((n -= 2 * sizeof(uint32_t)) <= 0)
    errx(1, "n = %d", n);

  fprintf(stdout, "From %s\n%s\n", sock_ntop(sa), sapp->sap_data);

  return(status);
}

const char *sock_ntop(struct sockaddr *sa){

  static char str[INET_ADDRSTRLEN];
  struct sockaddr_in *sin = (struct sockaddr_in *)sa;

  return(inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)));
}



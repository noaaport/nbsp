#include <sys/types.h>   /* for type definitions */
#include <sys/socket.h>  /* for socket API function calls */
#include <netinet/in.h>  /* for address structs */
#include <arpa/inet.h>   /* for sockaddr_in */
#include <stdio.h>       /* for printf() */
#include <stdlib.h>      /* for atoi() */
#include <string.h>      /* for strlen() */
#include <unistd.h>      /* for close() */

#define MAX_LEN  1024    /* maximum string size to send */
unsigned short gport = 1025;
char *gip[] = {"224.0.1.1", "224.0.1.2", "224.0.1.3", "224.0.1.4"};

int main(int argc, char *argv[]) {

  int sock[2];                   /* socket descriptor */
  char send_str[MAX_LEN];     /* string to send */
  struct sockaddr_in mc_addr[2]; /* socket address structure */
  unsigned int send_len;      /* length of string to send */
  char* mc_addr_str[2];          /* multicast IP address */
  unsigned short mc_port;     /* multicast port */
  unsigned char mc_ttl=1;     /* time to live (hop count) */
  int i;
  int N = 2;

  for(i = 0; i < N; ++i){
    mc_port = gport;
    /* create a socket for sending to the multicast address */
    if ((sock[i] = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("socket() failed");
      exit(1);
    }
  
    /* set the TTL (time to live/hop count) for the send */
    if ((setsockopt(sock[i], IPPROTO_IP, IP_MULTICAST_TTL, 
		    (void*) &mc_ttl, sizeof(mc_ttl))) < 0) {
      perror("setsockopt() failed");
      exit(1);
    } 
  
    /* construct a multicast address structure */
    memset(&mc_addr[i], 0, sizeof(struct sockaddr_in));
    mc_addr[i].sin_family      = AF_INET;
    mc_addr[i].sin_addr.s_addr = inet_addr(gip[i]);
    mc_addr[i].sin_port        = htons(mc_port);
  }

  printf("Begin typing (return to send, ctrl-C to quit):\n");

  /* clear send buffer */
  memset(send_str, 0, sizeof(send_str));

  while (fgets(send_str, MAX_LEN, stdin)) {
    send_len = strlen(send_str);

    for(i = 0; i < 2; ++i){
      /* send string to multicast address */
      if ((sendto(sock[i], send_str, send_len, 0, 
		  (struct sockaddr *) &mc_addr[i], 
		  sizeof(struct sockaddr_in))) != send_len) {
	perror("sendto() sent incorrect number of bytes");
	exit(1);
      }
    }
    /* clear send buffer */
    memset(send_str, 0, sizeof(send_str));
  }


  for(i = 0; i < N; ++i){
    close(sock[i]);  
  }

  exit(0);
}


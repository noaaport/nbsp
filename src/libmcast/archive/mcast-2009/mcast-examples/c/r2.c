#include <sys/types.h>  /* for type definitions */
#include <sys/socket.h> /* for socket API calls */
#include <netinet/in.h> /* for address structs */
#include <arpa/inet.h>  /* for sockaddr_in */
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() */
#include <string.h>     /* for strlen() */
#include <unistd.h>     /* for close() */
#include <err.h>

#define MAX_LEN  1024   /* maximum receive string size */

unsigned short gport = 1025;
char *gip[] = {"224.0.1.1", "224.0.1.2", "224.0.1.3", "224.0.1.4"};

main(int argc, char *argv[]) {

  int sock[2];                     /* socket descriptor */
  int flag_on = 1;              /* socket option flag */
  struct sockaddr_in mc_addr[2];   /* socket address structure */
  char recv_str[MAX_LEN+1];     /* buffer to receive string */
  int recv_len;                 /* length of string received */
  struct ip_mreq mc_req[2];        /* multicast request structure */
  char* mc_addr_str[2];            /* multicast IP address */
  unsigned short mc_port;       /* multicast port */
  struct sockaddr_in from_addr; /* packet source */
  unsigned int from_len;        /* source addr length */
  int i;
  int N = 2;
  int status;

  mc_port = gport;
  for(i = 0; i < N; ++i){
    mc_addr_str[i] = gip[i];

    /* create socket to join multicast group on */
    if ((sock[i] = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("socket() failed");
      exit(1);
    }
  
    /* set reuse port to on to allow multiple binds per host */
    if ((setsockopt(sock[i], SOL_SOCKET, SO_REUSEADDR, &flag_on,
		    sizeof(flag_on))) < 0) {
      perror("setsockopt() failed");
      exit(1);
    }

    /* construct a multicast address structure */
    memset(&mc_addr[i], 0, sizeof(struct sockaddr_in));
    mc_addr[i].sin_family      = AF_INET;
    mc_addr[i].sin_addr.s_addr = htonl(INADDR_ANY);
    mc_addr[i].sin_port        = htons(mc_port);

    /* bind to multicast address to socket */
    if ((bind(sock[i], (struct sockaddr *) &mc_addr[i], 
	      sizeof(struct sockaddr_in))) < 0) {
      fprintf(stderr, "%s ", mc_addr_str[i]);
      perror("bind() failed");
      exit(1);
    } else {
      fprintf(stderr, "%s bind() ok\n", mc_addr_str[i]);
    }

    /* construct an IGMP join request structure */
    mc_req[i].imr_multiaddr.s_addr = inet_addr(mc_addr_str[i]);
    mc_req[i].imr_interface.s_addr = htonl(INADDR_ANY);

    /* send an ADD MEMBERSHIP message via setsockopt */
    if ((setsockopt(sock[i], IPPROTO_IP, IP_ADD_MEMBERSHIP, 
		    (void*) &mc_req[i], sizeof(struct ip_mreq))) < 0) {
      perror("setsockopt() failed");
      exit(1);
    }
  }

  for (;;) {          /* loop forever */

    /* clear the receive buffers & structs */
    memset(recv_str, 0, sizeof(recv_str));
    from_len = sizeof(from_addr);
    memset(&from_addr, 0, from_len);

    for(i = 0; i < N; ++i){
      /* block waiting to receive a packet */
      if ((recv_len = recvfrom(sock[i], recv_str, MAX_LEN, 0, 
			       (struct sockaddr*)&from_addr, &from_len)) < 0) {
	perror("recvfrom() failed");
	exit(1);
      }
    }
    /* output received string */
    printf("%s: Received %d bytes from %s: ", mc_addr_str[i], recv_len, 
           inet_ntoa(from_addr.sin_addr));
    printf("%s: %s", mc_addr_str[i], recv_str);
  }
  /* send a DROP MEMBERSHIP message via setsockopt */
  for(i = 0; i < N; ++i){
    if ((setsockopt(sock[i], IPPROTO_IP, IP_DROP_MEMBERSHIP, 
		  (void*) &mc_req, sizeof(struct ip_mreq))) < 0) {
      perror("setsockopt() failed");
      exit(1);
    }
    
    close(sock[i]);
  }

  return(0);
}

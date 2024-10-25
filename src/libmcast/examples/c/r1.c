#include <sys/types.h>  /* for type definitions */
#include <sys/socket.h> /* for socket API calls */
#include <netinet/in.h> /* for address structs */
#include <arpa/inet.h>  /* for sockaddr_in */
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() */
#include <string.h>     /* for strlen() */
#include <unistd.h>     /* for close() */
#include <err.h>
#include <pthread.h>

#define MAX_LEN  1024   /* maximum receive string size */

unsigned short gport = 1025;
char *gip[] = {"224.0.1.1", "224.0.1.2", "224.0.1.3", "224.0.1.4"};

static int create_thread(void *arg);
static void *thread_main(void *arg);

main(int argc, char *argv[]) {

  int i;
  int N = 2;
  int status;

  for(i = 0; i < N; ++i){
    status = create_thread(gip[i]);
  }
  sleep(10);

  return(0);
}

static int create_thread(void *arg){

  int status = 0;
  pthread_attr_t attr;
  pthread_t thread_id;

  if(status == 0)
    status = pthread_attr_init(&attr);

  if(status == 0)
    status = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  if(status == 0)
    status = pthread_create(&thread_id, &attr, thread_main, arg);

  if(status != 0)
    errx(1, "Could not create thread: error %d.", status);

  return(status);
}

static void *thread_main(void *arg){

  int sock;                     /* socket descriptor */
  int flag_on = 1;              /* socket option flag */
  struct sockaddr_in mc_addr;   /* socket address structure */
  char recv_str[MAX_LEN+1];     /* buffer to receive string */
  int recv_len;                 /* length of string received */
  struct ip_mreq mc_req;        /* multicast request structure */
  char* mc_addr_str;            /* multicast IP address */
  unsigned short mc_port;       /* multicast port */
  struct sockaddr_in from_addr; /* packet source */
  unsigned int from_len;        /* source addr length */

  mc_addr_str = (char*)arg;
  mc_port = gport;

  /* create socket to join multicast group on */
  if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    perror("socket() failed");
    exit(1);
  }

  /* set reuse port to on to allow multiple binds per host */
#if defined(SO_REUSEPORT)
  if((setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &flag_on,
		 sizeof(flag_on))) < 0) {
#else
  if((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag_on,
		 sizeof(flag_on))) < 0) {
#endif
    perror("setsockopt() failed");
    exit(1);
  }

  /* Construct a multicast address structure. Bind to the multicast address */
  memset(&mc_addr, 0, sizeof(mc_addr));
  mc_addr.sin_family      = AF_INET;
  mc_addr.sin_addr.s_addr = inet_addr(mc_addr_str);
  mc_addr.sin_port        = htons(mc_port);

  /* bind to multicast address to socket */
  if ((bind(sock, (struct sockaddr *) &mc_addr, 
       sizeof(mc_addr))) < 0) {
    fprintf(stderr, "%s ", mc_addr_str);
    perror("bind() failed");
    exit(1);
  } else {
    fprintf(stderr, "%s bind() ok\n", mc_addr_str);
  }

  /* construct an IGMP join request structure */
  mc_req.imr_multiaddr.s_addr = inet_addr(mc_addr_str);
  mc_req.imr_interface.s_addr = htonl(INADDR_ANY);

  /* send an ADD MEMBERSHIP message via setsockopt */
  if ((setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
       (void*) &mc_req, sizeof(mc_req))) < 0) {
    perror("setsockopt() failed");
    exit(1);
  }

  for (;;) {          /* loop forever */

    /* clear the receive buffers & structs */
    memset(recv_str, 0, sizeof(recv_str));
    from_len = sizeof(from_addr);
    memset(&from_addr, 0, from_len);

    /* block waiting to receive a packet */
    if ((recv_len = recvfrom(sock, recv_str, MAX_LEN, 0, 
         (struct sockaddr*)&from_addr, &from_len)) < 0) {
      perror("recvfrom() failed");
      exit(1);
    }

    /* output received string */
    printf("%s: Received %d bytes from %s: ", mc_addr_str, recv_len, 
           inet_ntoa(from_addr.sin_addr));
    printf("%s: %s", mc_addr_str, recv_str);
  }

  /* send a DROP MEMBERSHIP message via setsockopt */
  if ((setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, 
       (void*) &mc_req, sizeof(mc_req))) < 0) {
    perror("setsockopt() failed");
    exit(1);
  }

  close(sock);

  return(NULL);
}

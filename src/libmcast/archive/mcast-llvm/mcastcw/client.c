#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int mcast_recv(char *multicastIP, char *multicastPort,
	       void **sa_ptr, socklen_t *sa_len){

  int status = 0;
  int fd = -1;
  unsigned short multicastPort;
  struct sockaddr_in multicastAddr; /* Multicast Address */
  struct ip_mreq multicastRequest;  /* Multicast address join structure */

  /*
   * Construct bind structure 
   */
  memset(&multicastAddr, 0, sizeof(multicastAddr));
  multicastAddr.sin_family = AF_INET;
  multicastAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  multicastAddr.sin_port = htons(multicastPort);
  
  /*
   * Fill in the mreq struct
   */
  multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastIP);
  multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
  
  if((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    return(-1);
  
  /* 
   * Bind to the multicast port
   */
  status = bind(fd, (struct sockaddr*)&multicastAddr, sizeof(multicastAddr));
  
  /*
   * Join the multicast address
   */
  if(status == 0)
    status = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			(void *)&multicastRequest, sizeof(multicastRequest));


  if(status == 0){
    *sa_ptr = malloc(sizeof(struct sockaddr_in));
    if(*sa_ptr != NULL){
      memcpy(*sa_ptr, &sa, sizeof(struct sockaddr_in));
      *sa_len = sizeof(struct sockaddr_in);
    }
  }

  if(status != 0){
    if(fd != -1){
      close(fd);
      fd = -1;
    }
  }
  
  return(fd);
}

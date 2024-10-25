    int main(void)
    {
      const int TTL=64;
      char buffer[BUFFSIZE];
      const int v_off=0, v_on=1;
      struct sockaddr_in to;
      struct ip_mreq multiaddr;
      int m;

      int sock = socket(AF_INET, SOCK_DGRAM, 0);

      // Join a multicast group 224.1.1.1 on all interfaces
      multiaddr.imr_multiaddr.s_addr = htonl(0xe0010104); // 224.1.1.1
      multiaddr.imr_interface.s_addr = htonl(INADDR_ANY);
      setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                     &multiaddr, sizeof(multiaddr));

      setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
                     &TTL, sizeof(TTL));

      // setup the destination address of 224.1.1.1 and UDP port 2007
      to.sin_family = AF_INET;
      to.sin_addr.s_addr = htonl(0xe0010104);
      to.sin_port = htons(2007);

      // write some data
      for(m=0;m<1000000;m++)
      {
        sendto(sock,buffer,BUFFSIZE, 0,
               (const struct sockaddr*)&to,sizeof(to));
      }

      // tell router we are no longer interested in this group
      close(sock);
    }

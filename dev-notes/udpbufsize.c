#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/*
 * See also
 *
 * http://www.29west.com/docs/THPM/udp-buffer-sizing.html
 */

int main(int argc, char **argv){

  int fd;
  int r;
  int old_r = 0;
  socklen_t r_size;
  int status = 0;
  int opt_tcp = 0;
  int opt_send = 0;
  int sock_opt_name = SO_RCVBUF;
  int c;
  char *optstr = "ts";   /* tcp, send buffer */
  char *usage = "udpsize [-t] [-s]";

  while((c = getopt(argc, argv, optstr)) != -1){
    switch(c){
    case 's':
      opt_send = 1;
      break;
    case 't':
      opt_tcp = 1;
      break;
    default:
      errx(1, "usage");
      break;
    }
  }

  if(opt_tcp)
    fd = socket(AF_INET, SOCK_STREAM, 0);
  else
    fd = socket(AF_INET, SOCK_DGRAM, 0);

  if(opt_send)
      sock_opt_name = SO_SNDBUF;

  if(fd == -1)
    err(1, "socket");

  r_size = (socklen_t)sizeof(int);
  status = getsockopt(fd, SOL_SOCKET, sock_opt_name, &r, &r_size);
  old_r = r;
  if(status == -1)
    err(1, "getsockopt");

  fprintf(stdout, "%d\n", r);

  while(status == 0){
    r += 1024;
    status = setsockopt(fd, SOL_SOCKET, sock_opt_name, &r, r_size);
    if(status != 0)
      err(1, "setsockopt");

    fprintf(stdout, "set %d: ", r);

    status = getsockopt(fd, SOL_SOCKET, sock_opt_name, &r, &r_size);
    if(status != 0)
      err(1, "getsockopt");

    fprintf(stdout, "get %d\n", r);

    if(r == old_r)
      break;
    else
      old_r = r;
  }

  close(fd);

  return(0);
}

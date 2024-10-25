#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <err.h>
#include <unistd.h>

int main(void){

  int fd;
  int r;
  int r_size;
  int status = 0;

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd == -1)
    err(1, "socket");

  r_size = sizeof(int);
  status = getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &r, &r_size);
  if(status == -1)
    err(1, "getsockopt");

  fprintf(stdout, "%d\n", r);

  r = 42080;
  status = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &r, r_size);
  if(status == -1)
    err(1, "setsockopt");

  status = getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &r, &r_size);
  if(status == -1)
    err(1, "getsockopt");

  fprintf(stdout, "%d\n", r);

  close(fd);

  return(0);
}

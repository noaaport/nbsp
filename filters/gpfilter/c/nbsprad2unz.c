/*
 * nbspdcrad2  [-h] | [-z] | <decode options> <file> | < <file>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>

#define BUFFER_SIZE 1000000

int main(int argc, char **argv){

  char *file ;
  int fd;
  char buffer[BUFFER_SIZE];
  int buffer_size = BUFFER_SIZE;
  int n;
  unsigned char *p;
  int size;
  int totalsize;
  int cut;

  if(argc < 3)
    errx(1, "argument");
  
  file = argv[1];
  cut = atoi(argv[2]);

  fd = open(file, O_RDONLY);
  if(fd == -1)
    err(1, "open");

  if((n = read(fd, buffer, buffer_size)) == -1)
    err(1, "read");

  close(fd);

  p = &buffer[cut];
  totalsize = 0;
  size = 0;
  do{
    fprintf(stdout, "totalsize: %d, size: %d\n", totalsize, size);
    size = abs((p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]);
    if(size == 0)
      break;

    p += size + 4;
    totalsize += size + 4;
  }while(totalsize < n);

  fprintf(stdout, "%d %d\n", n, totalsize);

  return(0);
}

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>

#define CCB0 64
#define CCB1 12

int has_ccb(char *data) {

  int r = 0;
  
  if((data[0] == CCB0) && (data[1] == CCB1))
    r = 1;

  return(r);
}

int main(int argc, char **argv) {

  int hasccb;
  int fd;
  int N = 512;
  char buffer[512];
  int n;
  
  if(argc < 2)
    errx(1, "%s", "file?");

  fd = open(argv[1], O_RDONLY);
  if(fd == -1)
    err(1, "open");

  n = read(fd, buffer, N);
  if(n == -1)
    err(1, "read");
  else if(n < 2)
    errx(1, "short file");

  hasccb = has_ccb(buffer);

  if(hasccb == 1)
    fprintf(stdout, "%s\n", "yes");
  else
    fprintf(stdout, "%s\n", "no");

  return(0);
}

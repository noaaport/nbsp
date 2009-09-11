/*
 * $Id$
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <err.h>

#define BODY_SIZE	512
#define CCB_SIZE	24

static int is_text(char *buffer, int size);

int main(int argc, char **argv){

  int body_size = BODY_SIZE;
  char body[BODY_SIZE];
  int size;
  int fd;
  int status;

  if(argc < 2)
    errx(1, "Needs argument.");

  fd = open(argv[1], O_RDONLY);
  if(fd == -1)
    err(1, "open()");

  size = read(fd, body, body_size);
  if(size < 0)
    err(1, "read()");

  close(fd);

  status = is_text(body, size);

  fprintf(stdout, "%d\n", status);

  return(0);
}

static int is_text(char *buffer, int size){

  int status = 0;
  int n;
  int c;

  if(size <= CCB_SIZE)
    return(1);

  size -= CCB_SIZE;
  buffer += CCB_SIZE;

  n = 0;
  while((n <= size - 1) && (status == 0)){
    c = *buffer;
    if((isprint(c) == 0) && (isspace(c) == 0)){
      status = 1;
      /*
       * According to the emwin documentation, these can appear
       *
      if((c == 0xc5) || (c == 0x80) | (c == 0x03) || (c == 0x83))
        status = 0;
      */
    }
    ++buffer;
    ++n;
  }

  return(status);
}

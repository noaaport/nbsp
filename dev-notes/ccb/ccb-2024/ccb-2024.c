#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>

int main(void){

  char *filename = "kgrr_sdus33-n1pgrr";
  int fd;
  unsigned char ccb[24];
  ssize_t n;
  unsigned char b1, b2;
  int size;
  unsigned char flag;
  unsigned char flag6_mask = (0xff >> 2);

  fd = open(filename, O_RDONLY);
  if(fd == -1)
    err(1, "%s", "open()");

  n = read(fd, ccb, 24);
  if(n != 24)
    err(1, "%s", "read()");

  close(fd);

  b1 = ccb[0];
  b2 = ccb[1];

  /* This shifts bits 7 and 8 to the positions 1 and 0. */
  flag = (b1 >> 6);
  fprintf(stdout, "flag = %#x\n", flag);

  /*
   * b2 is the low order byte, and bits 1-6 of bit make the high order byte.
   */
  size = b2 + ((b1 & (0xff >> 2)) << 8);
  fprintf(stdout, "pairs of bytes = %d\n", size);

  fprintf(stdout, "%c%c\n", b1, b2);
  fprintf(stdout, "%hhu %hhu\n", b1, b2);
  
	  
  return(0);
}


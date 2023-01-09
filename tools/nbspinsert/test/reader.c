#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define FINFO_BUFFER_SIZE 1024
static char finfo_buffer[FINFO_BUFFER_SIZE];

static uint32_t unpack_uint32(void *p, size_t start){
  /*
   * The first byte is the most significant
   * one and the last byte is the least significant.
   */
  uint32_t u;
  unsigned char *uptr;

  uptr = &((unsigned char*)p)[start];

  u = (uptr[0] << 24) + (uptr[1] << 16) + (uptr[2] << 8) + uptr[3];

  return(u);
}

int main(void) {

  int status = 0;
  int fd = -1;
  char *finfo = &finfo_buffer[0];
  unsigned char finfo_size_buffer[4];
  uint32_t finfo_size;
  int n;

  fd = open("infeed.fifo", O_RDWR);
  if(fd == -1)
    err(1, "%s", "Error from open");

  while(1) {
    n = read(fd, &finfo_size_buffer, 4);
    if(n < 4)
      err(1, "%s", "Error reading finfo_size");
    
    /* check */
    fprintf(stdout, "%d\n", n);
    
    finfo_size = unpack_uint32(finfo_size_buffer, 0);
    
    /* check */
    fprintf(stdout, "%u\n", finfo_size);
    
    n = read(fd, finfo, finfo_size + 1);
    
    /* check */
    fprintf(stdout, "%zu\n", strlen(finfo));

    /* check */
    if(finfo[finfo_size] != '\n')
      fprintf(stdout, "%s\n", "Last character is not cr");
    else
      finfo[finfo_size] = '\0';
    
    fprintf(stdout, "%s", finfo);

    fflush(stdout);
  }
  
  close(fd);

  return(0);
}

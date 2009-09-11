#include <stdlib.h>
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include "unz.h"

int main(int argc, char **argv){

  char *fin_name;
  char *fout_name;
  int fin_fd, fout_fd;
  char in_buffer[5200+1];
  int in_size = 5200+1;
  char out_buffer[5200+1];
  int out_size = 5200+1;
  int n;
  int status;

  if(argc != 3)
    errx(1, "argc");

  fin_name = argv[1];
  fout_name = argv[2];

  fin_fd = open(fin_name, O_RDONLY);
  if(fin_fd == -1)
    err(1, "open(fin)");

  fout_fd = open(fout_name, O_WRONLY | O_CREAT, 0644);
  if(fout_fd == -1)
    err(1, "open(fout)");

  n = read(fin_fd, in_buffer, in_size);
  if(n == -1)
    err(1, "read()");

  status = unz(out_buffer, &out_size, in_buffer, n);
  if(status != 0)
    errx(1, "unzip(): %d", status);

  n = write(fout_fd, out_buffer, out_size);
  if(n != out_size)
    err(1, "write()");

  close(fin_fd);
  close(fout_fd);

  return(0);
}


    
  

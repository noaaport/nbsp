#include <stdlib.h>
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include "unz.h"

int main(int argc, char **argv){

  char *fin_name;
  char *fout_name;
  int fin_fd, fout_fd;
  char in_buffer[4000];
  int in_size = 4000;
  char *out_buffer;
  int out_size;
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

  do{
    n = read(fin_fd, in_buffer, in_size);
    if(n == -1)
      err(1, "read()");
    else if(n > 0){

      status = zip(&out_buffer, &out_size, in_buffer, n);
      if(status != 0)
	err(1, "zip()");

      n = write(fout_fd, out_buffer, out_size);
      if(n != out_size)
	err(1, "write()");

      free(out_buffer);
    }
  }while(n > 0);

  close(fin_fd);
  close(fout_fd);

  return(0);
}


    
  

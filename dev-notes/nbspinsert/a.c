/*
 * Proof thet flock() does not work in freebsd on fifos
 *
 * ./read.sh
 * ./a.out
 */
#include <fcntl.h>
#include <err.h>
#include <unistd.h>

int main(void) {

  int fd = -1;
  int r;
  struct flock flock;

  flock.l_type = F_WRLCK;
  flock.l_start = 0;
  flock.l_whence = SEEK_SET;
  flock.l_len = 0;
 

  fd = open("infeed.fifo", O_WRONLY);
  if (fd == -1)
    err(1, "%s\n", "Open: ");

  r = fcntl(fd, F_SETLKW, &flock);
  if (r == -1)
    err(1, "%s\n", "fcntl: ");

  (void)close(fd);

  return(0);
}

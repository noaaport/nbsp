/*
 * Proof thet flock() works in freebsd on fifos
 *
 * ./read.sh (in one terminal)
 * ./b1 (in another terminal)
 * ./b2 (in another terminal)
 */
#include <string.h>
#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <sys/file.h>

int main(void) {

  int fd = -1;
  int r;
  char *s = "OK";
  
  fd = open("infeed.fifo", O_WRONLY);
  if (fd == -1)
    err(1, "%s\n", "Open: ");

  r = flock(fd, LOCK_EX);
  
  if (r == -1)
    err(1, "%s\n", "flock: ");

  sleep(10);
  write(fd, s, strlen(s));

  (void)close(fd);

  return(0);
}

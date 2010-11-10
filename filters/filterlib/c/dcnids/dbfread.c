/*
 * cc -o dbfread dbfread.c
 *
 * dbfread <station>.dbf
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <err.h>

int main(int argc, char **argv){

  char *fname;
  int fd;
  unsigned char b[128000];
  unsigned char *p;
  int n;
  int version, yy, mm, dd, numrecords, headerlength, recordlength;
  int numfields;
  int i;
  char *fieldname[2];
  int fieldlength;
  char code[4];
  char level[4];

  if(argc < 2)
    errx(1, "Needs file argument.");

  fname = argv[1];

  fd = open(fname, O_RDONLY);
  if(fd == -1)
    err(1, "open");

  if((n = read(fd, b, 128000)) == -1)
    err(1, "read");

  close(fd);

  version = b[0];
  yy = b[1];
  mm = b[2];
  dd = b[3];
  numrecords = b[4] + (b[5] << 8) + (b[6] << 16) + (b[7] << 24);
  headerlength = b[8] + (b[9] << 8);
  recordlength = b[10] + (b[11] << 8);

  numfields = (headerlength - 32 - 1)/32;

  fieldname[0] = (char*)&b[32];
  fieldname[1] = (char*)&b[64];

  fieldlength = b[64 + 16];

  fprintf(stdout, "%d %d %d %d %d %d %d %d %s %s\n",
	  version, yy, mm, dd, numrecords, headerlength, recordlength,
	  numfields, fieldname[0], fieldname[1]);

  p = &b[headerlength];

  memset(code, '\0', 4);
  memset(level, '\0', 4);
  for(i = 0; i < numrecords; ++i){
    ++p;
    memcpy(code, p, 3);
    p += 3;
    memcpy(level, p, 3);
    fprintf(stdout, "%s - %s\n", code, level);
    p += 3;
  }

  return(0);
}

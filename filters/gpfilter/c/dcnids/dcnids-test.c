#include <string.h>	/* memcpy */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include "dcnids_shp.h"

int main2(void){

  unsigned char buffer[2000000];
  unsigned char *b;
  int fd;
  int n, N;
  unsigned int record_offset, record_length, num_records;
  unsigned int file_length;

  fd = open("n0rjua.shx", O_RDONLY);
  if(fd == -1)
    err(1, "open");

  b = &buffer[0];

  if((N = read(fd, b, 2000000)) == -1)
    err(1, "read");

  close(fd);

  file_length = dcnids_shp_extract_uint32_big(b, 24);
  fprintf(stdout, "file length: %u, %d\n", file_length, N);

  b += 100;

  num_records = 0;
  n = 100;
  while(n < N){
    ++num_records;
    record_offset = dcnids_shp_extract_uint32_big(b, 0);
    record_length = dcnids_shp_extract_uint32_big(b, 4);
    b += 8;
    n += 8;
    fprintf(stdout, "%d: %u %u %u\n", n, num_records, record_offset,
	    record_length);
  }

  return(0);
}

int main(void){

  unsigned char buffer[2000000];
  unsigned char *b;
  int fd;
  int n, N;
  unsigned int record_number, record_length, num_records;
  unsigned int shapetype, numparts, numpoints, partindex;
  double x1, y1, x2, y2, x3, y3, x4, y4, x5, y5;

  fd = open("n0rjua.shp", O_RDONLY);
  if(fd == -1)
    err(1, "open");

  b = &buffer[0];

  if((N = read(fd, b, 2000000)) == -1)
    err(1, "read");

  close(fd);

  x1 = dcnids_shp_extract_double_little(b, 36);
  y1 = dcnids_shp_extract_double_little(b, 44);
  x2 = dcnids_shp_extract_double_little(b, 52);
  y2 = dcnids_shp_extract_double_little(b, 60);
  fprintf(stdout, "%f %f %f %f\n", x1, y1, x2, y2);

  b += 100;
  num_records = 0;
  n = 100;
  while(n < N){
    ++num_records;
    record_number = dcnids_shp_extract_uint32_big(b, 0);
    record_length = dcnids_shp_extract_uint32_big(b, 4);
    b += 8;
    n += 8;

    shapetype = dcnids_shp_extract_uint32_little(b, 0);
    numparts = dcnids_shp_extract_uint32_little(b, 36);
    numpoints = dcnids_shp_extract_uint32_little(b, 40);
    partindex = dcnids_shp_extract_uint32_little(b, 44);

    x1 = dcnids_shp_extract_double_little(b, 48);
    y1 = dcnids_shp_extract_double_little(b, 56);
    x2 = dcnids_shp_extract_double_little(b, 64);
    y2 = dcnids_shp_extract_double_little(b, 72);

    x3 = dcnids_shp_extract_double_little(b, 80);
    y3 = dcnids_shp_extract_double_little(b, 88);
    x4 = dcnids_shp_extract_double_little(b, 96);
    y4 = dcnids_shp_extract_double_little(b, 104);

    x5 = dcnids_shp_extract_double_little(b, 112);
    y5 = dcnids_shp_extract_double_little(b, 120);

    b += record_length * 2;
    n += record_length * 2;
 
    fprintf(stdout, "%d: %u %u %u %u %u ",
	    n, record_number, record_length,
	    shapetype, numparts, partindex);

    fprintf(stdout, "%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n",
	    x1, y1, x2, y2, x3, y3, x4, y4, x5, y5);
  }

  return(0);
}

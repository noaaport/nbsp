/*
 * Simple file to test the count_char() function used in the
 * validate_wmo() function, as explained in the file
 * validate_wmo.c
 */
#include <stdio.h>
#include <err.h>

#define N 32

int count_char(char *s, char c) {

  int i = 0;
  int count = 0;

  while((i < N) && (s[i] != '\n') && (s[i] != '\0')) {
    
    if(s[i] == c) {
      fprintf(stdout, "%c\n", s[i]);
      ++count;
    }

    ++i;
  }
  
  return(count);
}


int main(int argc, char **argv){

  char *line;
  int count;
  
  if(argc < 2)
    errx(1, "%s", "input?");

  line = argv[1];
  count = count_char(line, ' ');
  fprintf(stdout, "%d\n", count);

  return(0);
}

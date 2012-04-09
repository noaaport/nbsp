#include <stdio.h>
#include <err.h>

/*
 * From one of the messages in 1999-May.txt:
 * 
 * (indexes starting at 1)
 *
 * byte 15:	years since 1900
 *	16:	month (1-12)
 *	17:	day (1-31)
 *	18:	hour (00-23)
 *	19:	minute (11-59)
 *
 * It was verified using the program below, in a file
 *	tjsj_asca42-rwrpr.251710_69263187
 */

int main(int argc, char **argv){

  char *inputfile = argv[1];
  FILE *fin;
  unsigned char ccb[24];
  int year, month, day, hour, minute;

  fin = fopen(inputfile, "r");
  if(fin == NULL)
    err(1, "fopen()");

  if(fread(ccb, sizeof(char), 24, fin) != 24)
    err(1, "fread");
  
  fclose(fin);

  year = ccb[14];
  month = ccb[15];
  day = ccb[16];
  hour = ccb[17];
  minute = ccb[18];
  
  fprintf(stdout, "%d %d %d %d %d\n", year, month, day, hour, minute);

  return(0);
}

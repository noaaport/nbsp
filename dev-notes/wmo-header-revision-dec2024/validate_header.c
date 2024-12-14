/*
 * Dec 2024 - 
 * This is the test program for the function validate_wmo()
 * That was added in src/sbn.c to "validate"
 * the wmo header extracted from the file transmitted.
 * We used to "trust" the transmission, but in principle
 * we should check. I added this function as a complement to the
 * revision made for checking the presence of the "bbb", as explained
 * in the src/sbn.c file.
 *
 * This program is run by executing
 *
 * ./validate_wmo <filename>
 *
 * where <filename> is a standard file with a wmo header. Three sample
 * files are in the "samples" folder.
 */
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <err.h>

#define N 128
char gbuffer[N];

#define WMO_ID_SIZE 6
#define WMO_STATION_SIZE 4
#define WMO_TIME_SIZE 6
#define WMO_AWIPS_SIZE 6
#define WMO_NOTAWIPS_SIZE 6
#define WMO_BBB_SIZE 6

static int validate_wmo(char *wmo_id, char *wmo_station, char *wmo_time,
			char *wmo_bbb) {

  char *p;
  int len;

  /* check the length */
  if(strlen(wmo_id) != WMO_ID_SIZE)
    return(1);

  if(strlen(wmo_station) != WMO_STATION_SIZE)
    return(1);
  
  if(strlen(wmo_time) != WMO_TIME_SIZE)
    return(1);

  /* this can be blank
   * if(strlen(wmo_bbb) != WMO_BBB_SIZE)
   *   return(1);
   */
  
  /*check the content */
  p = wmo_id;
  while(*p != '\0'){
    if(isalnum(*p) == 0)
      return(1);

    ++p;
  }

  p = wmo_station;
  while(*p != '\0'){
    if(isalpha(*p) == 0)
      return(1);

    ++p;
  }

  p = wmo_time;
  while(*p != '\0'){
    if(isdigit(*p) == 0)
      return(1);

    ++p;
  }

  p = wmo_bbb;
  while(*p != '\0'){
    if(isalnum(*p) == 0)
      return(1);

    ++p;
  }

  return(0);
}
			
static int count_char(char *s, int n, char c) {

  int i = 0;
  int count = 0;

  while((i < n) && (s[i] != '\n') && (s[i] != '\0')) {
    
    if(s[i] == c)
      ++count;

    ++i;
  }
  
  return(count);
}

int split_wmo_header(char *wmo,
		     char *wmo_id, char *wmo_station, 
		     char *wmo_time, char *wmo_awips, char *wmo_notawips){
  char wmobbb[WMO_BBB_SIZE + 1];
  int data_size = N;	/* we can limit data_size to maybe 32 or so */
  int b;
  int i;
  int n;
  int wmo_blanks;

  /* In case we find nothing */
  memset(wmo_id, '\0', WMO_ID_SIZE);
  memset(wmo_station, '\0', WMO_STATION_SIZE);
  memset(wmo_time, '\0', WMO_TIME_SIZE);
  memset(wmo_awips, '\0', WMO_AWIPS_SIZE);
  memset(wmo_notawips, '\0', WMO_NOTAWIPS_SIZE);
  memset(wmobbb, '\0', WMO_BBB_SIZE);

  /* number of words in wmo */
  wmo_blanks = count_char(wmo, N, ' ');

  if(wmo_blanks == 3) {
    if(sscanf(wmo, "%6s %4s %6s %6s",
	      wmo_id, wmo_station, wmo_time, wmobbb) < 4)
      return(1);
  } else if(wmo_blanks == 2) {
    if(sscanf(wmo, "%6s %4s %6s",
	      wmo_id, wmo_station, wmo_time) < 3)
      return(1);
  } else {
    return(1);
  }

  if(validate_wmo(wmo_id, wmo_station, wmo_time, wmobbb) != 0)
    return(1);

  /*
   * start of the awips line, if any
   */
  i = 0;
  while((*wmo != '\n') && (i < data_size)){
     ++wmo;
     ++i;
  }

  if(i >= data_size - 1){
    /*
     * There is nothing.
     */
    return(0);
  }

  /*
   * Assuming we hit the \n, we have scanned (i + 1) characters,
   * including the \n. Thus he have to scan the rest, which are
   * data_size - (i + 1). First, point beyond the \n
   */

  ++wmo;

  /*
   * The line must contain at least the 6 characters of the awips
   * (some of them could be blanks) plus two \r plus a final \n
   * if it has an awips code, or some characters followed by a non
   * isalnum() character.
   */

  if(isalpha(wmo[0]) == 0){
    /*
     * There are no ascii characters. Copy the wmobbb (if there is one)
     * to the notawips.
     */
    b = 0;
    while((b < WMO_NOTAWIPS_SIZE) && (wmobbb[b] != '\0')){
      wmo_notawips[b] = wmobbb[b];
      ++b;
    }
    
    wmo_notawips[b] = '\0';
    
    return(0);
  }

  n = data_size - (i + 1);
  if((n >= 9) && (wmo[8] == '\n')){
    if(sscanf(wmo, "%6s", wmo_awips) != 1)
      wmo_awips[0] = '\0';
  }else{
    b = 0;
    while((b < WMO_NOTAWIPS_SIZE) && (b < n)){
      wmo_notawips[b] = wmo[b];

      if(isalnum(wmo_notawips[b]) == 0)
	break;
      
      ++b;
    }
    wmo_notawips[b] = '\0';
  }

  if((wmo_awips[0] == '\0') && (wmo_notawips[0] == '\0')){
    /* copy the wmo bbb (if it exists) to the notawips */
    b = 0;
    while((b < WMO_NOTAWIPS_SIZE) && (wmobbb[b] != '\0')){
      wmo_notawips[b] = wmobbb[b];
      ++b;
    }
    wmo_notawips[b] = '\0';
  }

  fprintf(stdout, "%s:%s:%s:%s\n", wmo_id, wmo_station, wmo_time, wmobbb);
    
  return(0);
}

int main(int argc, char **argv) {

  char *filename;
  char *wmo;
  int fd;
  int n;
  int b;
  char wmoid[WMO_ID_SIZE + 1];
  char wmostation[WMO_STATION_SIZE + 1];
  char wmotime[WMO_TIME_SIZE + 1];
  char awips[WMO_AWIPS_SIZE + 1];
  char nawips[WMO_NOTAWIPS_SIZE + 1];
  int status = 0;

  if(argc < 2)
    errx(1, "file?");

  filename = argv[1];
  fd = open(filename, O_RDONLY);
  if(fd == -1)
    err(1, "%s", "open");

  n = read(fd, gbuffer, N);
  if(n == -1)
    err(1, "%s", "read");

  if(close(fd) == -1)
    err(1, "%s", "close");

  /* get the keywords */  
  status = split_wmo_header(gbuffer,
			    wmoid, wmostation, 
			    wmotime, awips,
			    nawips);
  if(status != 0)
    errx(1, "%s", "split_wmo_header: no wmo line");

  fprintf(stdout, "%s:%s:%s\n", wmoid, wmostation, wmotime);
  fprintf(stdout, "%s:%s\n", awips, nawips);

  return(0);
}

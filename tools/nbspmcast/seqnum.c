/*
 * Copyright (c) 2024 Jose F. Nieves <nieves@ltp.uprrp.edu>
 */
#include <time.h> /* localtime */
#include <sys/time.h> /*gettineofday */
#include <inttypes.h>
#include <unistd.h>
#include "seqnum.h"

uint32_t get_seqnum_start(void) {
  /*
   * Returns the msecs since midnight. This will be used as the
   * seqnum. Of course it will reset every day.
   */
  struct timeval tv;
  struct timezone tz;
  time_t now_secs;
  struct tm *now_tm;
  uint32_t msecs; /* msecs since start of day */
    
  now_secs = time(NULL);
  now_tm = localtime(&now_secs);
  (void)gettimeofday(&tv, &tz);

  msecs = (now_tm->tm_hour * 3600 + now_tm->tm_min * 60 + now_tm->tm_sec)*1000;
  msecs += tv.tv_usec/1000;

  return(msecs);
}

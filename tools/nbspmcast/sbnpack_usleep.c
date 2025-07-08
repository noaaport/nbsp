/*
 * Copyright (c) 2025 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <unistd.h>	/* usleep */
#include "sbnpack_usleep.h"

#define MAX_FRAMES_PER_MS 1000		/* 4 GB/s. 32 Gb/s */
#define MIN_FRAMES_PER_MS 1		/* 4 MB/s, 32 Mb/s */
#define DEFAULT_FRAMES_PER_MS 10	/* 40 MB/s, 320 Mb/s */

/* static variable */
static useconds_t s_usecs = 0;

void init_sendto_sbnpack_usleep(int frames_per_ms) {
  /*
   * frames_per_ms = -1 => default
   * frames_per_ms = 0  => useconds = 0
   */
  useconds_t useconds;
  
  /* set default - used if the function argument is negative */
  useconds = 1000/DEFAULT_FRAMES_PER_MS;

  if(frames_per_ms == 0)
     useconds = 0;
  else if (frames_per_ms > 0) {
    if(frames_per_ms > MAX_FRAMES_PER_MS)
      frames_per_ms = MAX_FRAMES_PER_MS;

    useconds = (useconds_t)(1000/frames_per_ms);
  }
  
  s_usecs = useconds;
}

void sendto_sbnpack_usleep(void) {

  usleep(s_usecs);
}

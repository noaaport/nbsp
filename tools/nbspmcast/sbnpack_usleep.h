/*
 * Copyright (c) 2025 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef SBNPACK_USLEEP_H
#define SBNPACK_USLEEP_H

/*
 * frames_per_ms = -1 => use the default (10) frames/ms;
 * the corresponding useconds for usleep is 0.1 ms or 100 usecs;
 * corresponds to about 4KB * 10 * 1000/s = 40 MB/s
 *
 * frames_per_ms = 0  => use usleep with useconds = 0
 */
void init_sendto_sbnpack_usleep(int frames_per_ms);
void sendto_sbnpack_usleep(void);

#endif

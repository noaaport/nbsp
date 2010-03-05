/*
 * Copyright (c) 2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef CONFIG_H
#define CONFIG_H

/*
 * defaults (FreeBSD)
 */
#define HAVE_OPTRESET

#ifdef __OpenBSD__
  #define HAVE_OPTRESET
#elif defined __sun__
  #undef HAVE_OPTRESET
#elif defined __linux__
  #undef HAVE_OPTRESET
#endif
 
#endif

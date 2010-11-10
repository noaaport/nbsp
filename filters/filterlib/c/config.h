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
#define HAVE_ERR

#if defined __linux__
  #undef HAVE_OPTRESET
#elif defined __sun__
  #undef HAVE_OPTRESET
  #undef HAVE_ERR
#endif
 
#endif

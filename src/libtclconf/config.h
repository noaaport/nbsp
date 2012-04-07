/*
 * $Id$
 */
#ifndef CONFIG_H
#define CONFIG_H

/*
 * defaults (FreeBSD|OpenBSD|Linux)
 */
#define HAVE_ERR

#if defined __sun__
  #undef HAVE_ERR
#endif

#endif

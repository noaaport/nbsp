/*
 * $Id$
 */
#ifndef CONFIG_H
#define CONFIG_H

/*
 * defaults (FreeBSD|OpenBSD|Linux)
 */
#define HAVE_SUN_LEN
#define PID_T_INT

#if defined __sun__
  #undef  HAVE_SUN_LEN
  #undef  PID_T_INT
#endif

#endif
  


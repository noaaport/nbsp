/*
 * $Id$
 */
#ifndef CONFIG_H
#define CONFIG_H

/*
 * defaults (FreeBSD)
 */
#define HAVE_DAEMON
#define HAVE_LIBUTIL_H
#undef  HAVE_UTIL_H
#define HAVE_ERR
#define HAVE_ATON
#undef  HAVE_PTON

#ifdef __OpenBSD__
  #undef  HAVE_LIBUTIL_H
  #define HAVE_UTIL_H
#elif defined __sun__
  #undef HAVE_DAEMON
  #undef HAVE_LIBUTIL_H
  #undef HAVE_UTIL_H
  #undef HAVE_ERR
  #undef HAVE_ATON
  #define HAVE_PTON
  #define SOLARIS
#elif defined __linux__
  #undef HAVE_LIBUTIL_H
  #undef HAVE_UTIL_H
#endif

 
#endif

/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include "pw.h"

int get_user_uid(char *name, uid_t *uid){

  struct passwd *p;

  p = getpwnam(name);
  if(p == NULL)
    return(-1);

  *uid = p->pw_uid;

  return(0);
}

int get_group_gid(char *name, gid_t *gid){

  struct group *g;

  g = getgrnam(name);
  if(g == NULL)
    return(-1);

  *gid = g->gr_gid;

  return(0);
}

int change_user(char *name){

  uid_t uid;
  int status = 0;

  status = get_user_uid(name, &uid);
  if(status == 0)
    status = setuid(uid);

  return(status);
}

int change_group(char *name){

  gid_t gid;
  int status = 0;

  status = get_group_gid(name, &gid);
  if(status == 0)
    status = setgid(gid);

  return(status);
}


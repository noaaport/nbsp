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
#include <stdlib.h>
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

int change_groups(int ngroups, char **groups){

  gid_t *gid;
  int status = 0;
  int i;

  status = change_group(groups[0]);
  if(status != 0)
    return(-1);

  --ngroups;
  ++groups;

  if(ngroups == 0)
    return(0);

  gid = malloc(sizeof(gid_t)*ngroups);
  if(gid == NULL)
    return(-1);

  for(i = 0; i < ngroups; ++i){
    status = get_group_gid(groups[i], &gid[i]);
    if(status != 0)
      break;
  }

  if(status == 0)
    status = setgroups(ngroups, gid);

  free(gid);

  return(status);
}

int change_privs(int ngroups, char **groups, char *user, char *home){
  /*
   * Returns:
   *
   * -1 => cannot change user
   * -2 => cannot change group
   * -3 => cannot change home
   */

  int status = 0;

  /*
   * Change the group(s) first.
   */
  if(groups != NULL){
    status = change_groups(ngroups, groups);
    if(status != 0)
      return(-2);
  }

  if(user != NULL){
    status = change_user(user);
    if(status != 0)
      return(-1);
  }

  /*
   * This is needed when running as a normal user since otherwise
   * it cannot dump core (to "/") if it has to.
   */
  if(home != NULL){
    status = chdir(home);
    if(status != 0)
      return(-3);
  }

  return(0);
}

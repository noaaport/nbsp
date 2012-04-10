/*
 * Copyright (c) 2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#ifndef PW_H
#define PW_H

#include <sys/types.h>

int get_user_uid(char *name, uid_t *uid);
int get_group_gid(char *name, gid_t *gid);
int change_user(char *name);
int change_group(char *name);
int change_groups(int numnames, char **names);
int change_privs(int ngroups, char **groups, char *user, char *home);

#endif

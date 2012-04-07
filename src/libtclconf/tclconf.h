/*
 * Copyright (c) 2004 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */

#ifndef TCLCONF_H
#define TCLCONF_H

struct confoption_st {
  char *name;
  int id;
  int type;
  char *p;
  int v;
};  

int parse_configfile(char *scriptname, struct confoption_st *opt);
int parse_configfile2(char *scriptname, struct confoption_st *opt);
void kill_confopt_table(struct confoption_st *opt);
struct confoption_st *find_confoption(struct confoption_st *opt, int id);
void setoptval(void *var, struct confoption_st *optable, int id);

#endif

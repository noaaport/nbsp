#include <stdio.h>
#include <err.h>
#include <string.h>
#include "nbspdb.h"

static struct nbspdb_prec_st gprec[] = {
  {{0}, {9876, 1}},
  {{0}, {5287, 0}},
  {{0}, {9342, 1}},
  {{0}, {0, 0}}
};

void init(void){

  strcpy(gprec[0].fname, "tjsj_asca42");
  strcpy(gprec[1].fname, "kwbc_smf65");
  strcpy(gprec[2].fname, "knes_tig32");
}

static int init_records(struct nbspdb_st *nbspdb, struct nbspdb_prec_st *prec);

int main(void){

  int status;
  struct nbspdb_st *nbspdb;

  init();

  status = nbspdb_open(&nbspdb, "db.db", "sdb.db", DB_CREATE);
  if(status != 0)
    errx(1, "nbspdb_open: %d", status);

  if(status == 0)
    status = init_records(nbspdb, gprec);

  if(status != 0)
    errx(1, "init_records");

  if(nbspdb != NULL)
    nbspdb_close(nbspdb);

  return(0);
}

static int init_records(struct nbspdb_st *nbspdb, struct nbspdb_prec_st *prec){

  int status = 0;
  struct nbspdb_prec_st *p = prec;

  while((p->fname[0] != '\0') && (status == 0)){
    status = nbspdb_put(nbspdb, p);
    ++p;
  }

  return(status);
}

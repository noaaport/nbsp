#include <stdio.h>
#include <err.h>
#include <string.h>
#include "qdb.h"

#define FNAME_SIZE 18

struct nbspdb_prec_st{
  char fname[FNAME_SIZE + 1];
  u_int32_t seq;
  int status;
};

static struct nbspdb_prec_st gprec[] = {
  {{0}, 9876, 1},
  {{0}, 5287, 0},
  {{0}, 9342, 1},
  {{0}, 0, 0}
};

void init(void){

  strcpy(gprec[0].fname, "tjsj_asca42");
  strcpy(gprec[1].fname, "kwbc_smf65");
  strcpy(gprec[2].fname, "knes_tig32");
}

static int init_records(struct nbspqdb_st *nqdb, struct nbspdb_prec_st *prec);

int main(void){

  struct nbspqdb_st *nqdb;
  int status;

  init();

  status = qdb_open(&nqdb, "db.db", sizeof(struct nbspdb_prec_st));
  if(status != 0)
    errx(1, "nbspdb_open: %d", status);

  if(status == 0)
    status = init_records(nqdb, gprec);

  if(status != 0)
    errx(1, "init_records");

  /*
  print_all_records(nqdb);
  */

  if(nqdb != NULL)
    qdb_close(nqdb);

  return(0);
}

static int init_records(struct nbspqdb_st *nqdb, struct nbspdb_prec_st *prec){

  int status = 0;
  struct nbspdb_prec_st *p = prec;
  
  while((p->fname[0] != '\0') && (status == 0)){
    status = qdb_snd(nqdb, p, sizeof(struct nbspdb_prec_st));
    ++p;
  }

  return(status);
}


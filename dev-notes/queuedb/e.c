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

static void print_all_records(struct nbspqdb_st *nqdb);

int main(void){

  struct nbspqdb_st *nqdb;
  int status;
  u_int32_t dbflags = QDB_FLAGS;

  status = qdb_open(&nqdb, "db.db", sizeof(struct nbspdb_prec_st));
  if(status != 0)
    errx(1, "nbspdb_open: %d", status);

  print_all_records(nqdb);

  qdb_close(nqdb);

  return(0);
}

static void print_all_records(struct nbspqdb_st *nqdb){

  int status;
  struct nbspdb_prec_st prec;
  u_int32_t size;

  size = sizeof(prec);
  fprintf(stdout, "%u\n", size);

  while((status = qdb_rcv(nqdb, &prec, size)) == 0){
    fprintf(stdout, "%s %u %d\n", prec.fname, prec.seq, prec.status);
  }

  if(status != 0)
    warnx(db_strerror(status));
}


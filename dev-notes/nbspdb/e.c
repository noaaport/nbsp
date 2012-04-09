#include <stdio.h>
#include <err.h>
#include <string.h>
#include "nbspdb.h"

static void print_all_records(DB *dbp);
static void sprint_all_records(DB *sdbp);

int main(void){

  struct nbspdb_st *nbspdb;
  int status;
  struct nbspdb_prec_st prec;

  status = nbspdb_open(&nbspdb, "db.db", "sdb.db", DB_RDONLY);
  if(status != 0)
    errx(1, "nbspdb_open: %d", status);

  strcpy(prec.fname, "tjsj_asca42");
  status = nbspdb_get_byname(nbspdb, &prec);
  if(status != 0){
    (nbspdb->dbp)->err(nbspdb->dbp, status, "error");
    errx(1, "read_record: %d", status);
  }

  fprintf(stdout, "%s %u %d\n", prec.fname, prec.data.seq, prec.data.status);

  print_all_records(nbspdb->dbp);
  sprint_all_records(nbspdb->sdbp);  

  prec.data.seq = 9876;
  status = nbspdb_get_byseq(nbspdb, &prec);
  if(status != 0){
    (nbspdb->sdbp)->err(nbspdb->sdbp, status, "error");
    errx(1, "read_record: %d", status);
  }

  fprintf(stdout, "%s %u %d\n", prec.fname, prec.data.seq, prec.data.status);

  strcpy(prec.fname, "tjsj_asca42");
  prec.data.seq = 12345678; 
  status = nbspdb_update(nbspdb, &prec);
  print_all_records(nbspdb->dbp);
  sprint_all_records(nbspdb->sdbp);  

  status = nbspdb_put2(nbspdb, "kakakaka", 999999, 2);
  status= nbspdb_update2(nbspdb, "tjsj_asca42", 888888, 3);
  print_all_records(nbspdb->dbp);

  nbspdb_close(nbspdb);

  return(0);
}

static void print_all_records(DB *dbp){

  DBC *cursor;
  DBT key, data;
  int status;
  char *fname;
  struct nbspdb_pdata_st *pdata;

  dbp->cursor(dbp, NULL, &cursor, 0);
  memset(&key, 0, sizeof(DBT));
  memset(&data, 0, sizeof(DBT));

  while((status = cursor->c_get(cursor, &key, &data, DB_NEXT)) == 0){
    fname = key.data;
    pdata = data.data;
    fprintf(stdout, "%s %u %d\n", fname, pdata->seq, pdata->status);
  }
}

static void sprint_all_records(DB *sdbp){

  DBC *cursor;
  DBT key, data;
  int status;
  unsigned int *seq;
  struct nbspdb_pdata_st *pdata;

  sdbp->cursor(sdbp, NULL, &cursor, 0);
  memset(&key, 0, sizeof(DBT));
  memset(&data, 0, sizeof(DBT));

  while((status = cursor->c_get(cursor, &key, &data, DB_NEXT)) == 0){
    seq = key.data;
    pdata = data.data;
    fprintf(stdout, "%u %u %d\n", *seq, pdata->seq, pdata->status);
  }
}

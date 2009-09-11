#include <stdio.h>
#include <err.h>
#include <string.h>
#include "nbspdb.h"

/*
 * The received files (nbspdb) database.
 */
#define NBSPDB_PRIMARY_FNAME    "/var/noaaport/nbsp/nbsp.pdb"
#define NBSPDB_SECONDARY_FNAME  "/var/noaaport/nbsp/nbsp.sdb"

static void print_all_records(DB *dbp);

int main(void){

  struct nbspdb_st *nbspdb;
  int status;

  status = nbspdb_open(&nbspdb, NBSPDB_PRIMARY_FNAME, NBSPDB_SECONDARY_FNAME,
		       DB_RDONLY);
  if(status != 0)
    errx(1, "nbspdb_open: %s", db_strerror(status));

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


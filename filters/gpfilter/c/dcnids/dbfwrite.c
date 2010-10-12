/*
 * $Id$
 *
 *
 *  cc -o dbfcreate -I /usr/local/include -L /usr/local/lib -lshp dbf.c
 */
#include <shapefil.h>
#include <stdio.h>
#include <err.h>

int main(int argc, char **argv){

  DBFHandle dbfh;
  int i;
  int code;

  if(argc == 1)
    errx(1, "Needs output file argument.");

  dbfh = DBFCreate(argv[1]);
  if(dbfh == NULL)
    err(1, "DBFCreate");

  if(DBFAddField(dbfh, "code", FTInteger, 2, 0) == -1)
    errx(1, "DBFAddField");

  i = 0;
  while(fscanf(stdin, "%d", &code) > 0){
    if(DBFWriteIntegerAttribute(dbfh, i, 0, code) == 0)
      errx(1, "DBFWriteIntegerAttribute");

    ++i;
  }

  DBFClose(dbfh);

  return(0);
}

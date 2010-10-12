/*
 * $Id$
 *
 *
 *  cc -c <name> -I /usr/local/include -L /usr/local/lib -lshp
 */
#include <shapefil.h>
#include "dcnids.h"

int dcnids_dbf_write(char *dbfname,
		     char *paramname,
		     struct dcnids_polygon_map_st *pm){

  DBFHandle dbfh;
  int i;

  dbfh = DBFCreate(dbfname);
  if(dbfh == NULL)
    return(-1);

  if(DBFAddField(dbfh, paramname, FTInteger, 2, 0) == -1)
    return(1);

   for(i = 0; i < pm->numpolygons; ++i){
    if(DBFWriteIntegerAttribute(dbfh, i, 0, pm->polygons[i].code) == 0){
      DBFClose(dbfh);
      return(2);
    }
  }

  DBFClose(dbfh);

  return(0);
}

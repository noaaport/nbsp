/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdlib.h>
#include <stdio.h>
#include "err.h"
#include "dcgini.h"
#include "dcgini_shp.h"		/* some of the insert_uintxx functions */

struct dcgini_dbfinfo_st {
  int numfields;
  int fieldwidth;
  int numrecords;
};

struct dcgini_dbf_st {
  unsigned char *b;
  int size;
  struct dcgini_dbfinfo_st dbfinfo;
};

#define DCGINI_DBF_HEADER_SIZE		32
#define DCGINI_DBF_FIELD_DESC_SIZE	32
#define DCGINI_DBF_TERMINATOR		'\015'
#define DCGINI_DBF_FIELDNAME_LEN	10	/* excluding '\0' */

#define DCGINI_DBF_FIELDWIDTH		3
#define DCGINI_DBF_NUMFIELDS		1
#define DCGINI_DBF_FIELDNAME		"level"

#define DCGINI_DBF_RECORD_VALID_FLAG	0x20  /* blank => valid record */

static void dcgini_dbf_insert_uint16_little(unsigned char*b,
					    int pos, uint16_t u);
static int dcgini_dbf_total_size_bytes(struct dcgini_dbfinfo_st *dbfinfo);
static int dcgini_dbf_record_size_bytes(struct dcgini_dbfinfo_st *dbfinfo);
static int dcgini_dbf_header_size_bytes(struct dcgini_dbfinfo_st *dbfinfo);

static struct dcgini_dbf_st *dcgini_dbf_create(int numrecords);
static void dcgini_dbf_destroy(struct dcgini_dbf_st *dbf);
static void dcgini_dbf_insert_content(struct dcgini_dbf_st *dbf,
				      struct dcgini_point_map_st *pm);

static void dcgini_dbf_insert_uint16_little(unsigned char *p,
					    int pos, uint16_t u){

  unsigned char *b = p;

  b += pos;

  b[0] = u & 0xff;
  b[1] = (u >> 8) & 0xff;
}

static int dcgini_dbf_record_size_bytes(struct dcgini_dbfinfo_st *dbfinfo){

  int recordsize;

  /* include the deleted flag */
  recordsize = 1 + dbfinfo->fieldwidth * dbfinfo->numfields;	

  return(recordsize);
}

static int dcgini_dbf_header_size_bytes(struct dcgini_dbfinfo_st *dbfinfo){
  /*
   * This the "header length", which is everything up to the
   * start of the records:  header, fields descriptor array, terminator.
   */
  int headersize;

  /* includes 1 for the terminator after the fields descriptor array */ 
  headersize = DCGINI_DBF_HEADER_SIZE +
    dbfinfo->numfields * DCGINI_DBF_FIELD_DESC_SIZE + 1;
  
  return(headersize);
}

static int dcgini_dbf_total_size_bytes(struct dcgini_dbfinfo_st *dbfinfo){

  int recordsize, totalsize;

  recordsize = dcgini_dbf_record_size_bytes(dbfinfo);

  totalsize = dcgini_dbf_header_size_bytes(dbfinfo) +
    recordsize * dbfinfo->numrecords;

  /*
   * Add one to make easier for snprintf of the last record to insert
   * the terminating '\0'.
   */
  ++totalsize;

  return(totalsize);
}

static struct dcgini_dbf_st *dcgini_dbf_create(int numrecords){

  struct dcgini_dbf_st *dbf;
  int totalsize;

  dbf = malloc(sizeof(struct dcgini_dbf_st));
  if(dbf == NULL)
    return(NULL);

  dbf->dbfinfo.numfields = DCGINI_DBF_NUMFIELDS;
  dbf->dbfinfo.fieldwidth = DCGINI_DBF_FIELDWIDTH;
  dbf->dbfinfo.numrecords = numrecords;

  totalsize = dcgini_dbf_total_size_bytes(&dbf->dbfinfo);
  dbf->b = calloc(totalsize, sizeof(char));
  if(dbf->b == NULL){
    free(dbf);
    return(NULL);
  }
  dbf->size = totalsize;

  return(dbf);
}

static void dcgini_dbf_destroy(struct dcgini_dbf_st *dbf){

  if(dbf->b != NULL)
    free(dbf->b);

  free(dbf);
}

HERE
static void dcgini_dbf_insert_content(struct dcgini_dbf_st *dbf,
				      struct dcgini_polygon_map_st *pm){
  unsigned char *b;

  uint16_t recordsize;
  uint16_t headersize;
  int n;
  int i;
  
  recordsize = (uint16_t)dcgini_dbf_record_size_bytes(&dbf->dbfinfo);
  headersize = (uint16_t)dcgini_dbf_header_size_bytes(&dbf->dbfinfo);

  /* Header */
  dcgini_shp_insert_uint32_little(dbf->b, 4, 
				  (uint32_t)dbf->dbfinfo.numrecords);
  dcgini_dbf_insert_uint16_little(dbf->b, 8, headersize);
  dcgini_dbf_insert_uint16_little(dbf->b, 10, recordsize);

  /* Field descriptor array */
  b = &dbf->b[32];
  strncpy((char*)b, DCGINI_DBF_FIELDNAME_0, DCGINI_DBF_FIELDNAME_LEN);
  b[11] = 'N';
  b[16] = DCGINI_DBF_FIELDWIDTH;
  b[17] = 0;	/* decimal count */
  b += 32;

  strncpy((char*)b, DCGINI_DBF_FIELDNAME_1, DCGINI_DBF_FIELDNAME_LEN);
  b[11] = 'N';
  b[16] = DCGINI_DBF_FIELDWIDTH;
  b[17] = 0;	/* decimal count */
  b += 32;

  /* terminator */
  b[0] = 0xd;
  ++b;		

  /*
   * records
   */
  for(i = 0; i < pm->numpolygons; ++i){
    b[0] = DCGINI_DBF_RECORD_VALID_FLAG;
    ++b;
    /*
     * In the calculation of the totalsize we must ensure that the
     * last record has enough room for the terminating '\0'.
     */
    n = snprintf((char*)b, 2*DCGINI_DBF_FIELDWIDTH + 1, "%3d%3d",
		 pm->polygons[i].code, pm->polygons[i].level);

    assert(n == 2*DCGINI_DBF_FIELDWIDTH);

    /*
     * This will overwite the terminating '\0' of the previous record,
     * which is how it should be.
     */
    b += 2*DCGINI_DBF_FIELDWIDTH;
  }
}

int dcgini_dbf_write(char *file, struct dcgini_point_map_st *pm){

  int fd;
  int n;
  struct dcgini_dbf_st *dbf;
  int status = 0;

  dbf = dcgini_dbf_create(pm->numpolygons);
  if(dbf == NULL)
    return(-1);

  dcgini_dbf_insert_content(dbf, pm);

  fd = open(dbfname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd == -1){
    status = -1;
    goto End;
  }

  n = write(fd, dbf->b, dbf->size);
  if(n < 0)
    status = -1;

 End:
    dcgini_dbf_destroy(dbf);

    return(status);
}

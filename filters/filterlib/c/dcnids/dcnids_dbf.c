/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "dcnids.h"
#include "dcnids_shp.h"		/* some of the insert_uintxx functions */

struct dcnids_dbfinfo_st {
  int numfields;
  int fieldwidth;
  int numrecords;
};

struct dcnids_dbf_st {
  unsigned char *b;
  int size;
  struct dcnids_dbfinfo_st dbfinfo;
};

#define DCNIDS_DBF_HEADER_SIZE		32
#define DCNIDS_DBF_FIELD_DESC_SIZE	32
#define DCNIDS_DBF_TERMINATOR		'\015'
#define DCNIDS_DBF_FIELDNAME_LEN	10	/* excluding '\0' */

#define DCNIDS_DBF_FIELDWIDTH		3
#define DCNIDS_DBF_NUMFIELDS		2
#define DCNIDS_DBF_FIELDNAME_0		"code"
#define DCNIDS_DBF_FIELDNAME_1		"level"

#define DCNIDS_DBF_RECORD_VALID_FLAG	0x20  /* blank => valid record */

static void dcnids_dbf_insert_uint16_little(unsigned char*b,
					    int pos, uint16_t u);
static int dcnids_dbf_total_size_bytes(struct dcnids_dbfinfo_st *dbfinfo);
static int dcnids_dbf_record_size_bytes(struct dcnids_dbfinfo_st *dbfinfo);
static int dcnids_dbf_header_size_bytes(struct dcnids_dbfinfo_st *dbfinfo);

static struct dcnids_dbf_st *dcnids_dbf_create(int numrecords);
static void dcnids_dbf_destroy(struct dcnids_dbf_st *dbf);
static void dcnids_dbf_insert_content(struct dcnids_dbf_st *dbf,
				      struct dcnids_polygon_map_st *pm);

static void dcnids_dbf_insert_uint16_little(unsigned char *p,
					    int pos, uint16_t u){

  unsigned char *b = p;

  b += pos;

  b[0] = u & 0xff;
  b[1] = (u >> 8) & 0xff;
}

static int dcnids_dbf_record_size_bytes(struct dcnids_dbfinfo_st *dbfinfo){

  int recordsize;

  /* include the deleted flag */
  recordsize = 1 + dbfinfo->fieldwidth * dbfinfo->numfields;	

  return(recordsize);
}

static int dcnids_dbf_header_size_bytes(struct dcnids_dbfinfo_st *dbfinfo){
  /*
   * This the "header length", which is everything up to the
   * start of the records:  header, fields descriptor array, terminator.
   */
  int headersize;

  /* includes 1 for the terminator after the fields descriptor array */ 
  headersize = DCNIDS_DBF_HEADER_SIZE +
    dbfinfo->numfields * DCNIDS_DBF_FIELD_DESC_SIZE + 1;
  
  return(headersize);
}

static int dcnids_dbf_total_size_bytes(struct dcnids_dbfinfo_st *dbfinfo){

  int recordsize, totalsize;

  recordsize = dcnids_dbf_record_size_bytes(dbfinfo);

  totalsize = dcnids_dbf_header_size_bytes(dbfinfo) +
    recordsize * dbfinfo->numrecords;

  /*
   * Add one to make easier for snprintf of the last record to insert
   * the terminating '\0'.
   */
  ++totalsize;

  return(totalsize);
}

static struct dcnids_dbf_st *dcnids_dbf_create(int numrecords){

  struct dcnids_dbf_st *dbf;
  int totalsize;

  dbf = malloc(sizeof(struct dcnids_dbf_st));
  if(dbf == NULL)
    return(NULL);

  dbf->dbfinfo.numfields = DCNIDS_DBF_NUMFIELDS;
  dbf->dbfinfo.fieldwidth = DCNIDS_DBF_FIELDWIDTH;
  dbf->dbfinfo.numrecords = numrecords;

  totalsize = dcnids_dbf_total_size_bytes(&dbf->dbfinfo);
  dbf->b = calloc(totalsize, sizeof(char));
  if(dbf->b == NULL){
    free(dbf);
    return(NULL);
  }
  dbf->size = totalsize;

  return(dbf);
}

static void dcnids_dbf_destroy(struct dcnids_dbf_st *dbf){

  if(dbf->b != NULL)
    free(dbf->b);

  free(dbf);
}

static void dcnids_dbf_insert_content(struct dcnids_dbf_st *dbf,
				      struct dcnids_polygon_map_st *pm){
  unsigned char *b;

  uint16_t recordsize;
  uint16_t headersize;
  int n;
  int i;
  
  recordsize = (uint16_t)dcnids_dbf_record_size_bytes(&dbf->dbfinfo);
  headersize = (uint16_t)dcnids_dbf_header_size_bytes(&dbf->dbfinfo);

  /* Header */
  dcnids_shp_insert_uint32_little(dbf->b, 4, 
				  (uint32_t)dbf->dbfinfo.numrecords);
  dcnids_dbf_insert_uint16_little(dbf->b, 8, headersize);
  dcnids_dbf_insert_uint16_little(dbf->b, 10, recordsize);

  /* Field descriptor array */
  b = &dbf->b[32];
  strncpy((char*)b, DCNIDS_DBF_FIELDNAME_0, DCNIDS_DBF_FIELDNAME_LEN);
  b[11] = 'N';
  b[16] = DCNIDS_DBF_FIELDWIDTH;
  b[17] = 0;	/* decimal count */
  b += 32;

  strncpy((char*)b, DCNIDS_DBF_FIELDNAME_1, DCNIDS_DBF_FIELDNAME_LEN);
  b[11] = 'N';
  b[16] = DCNIDS_DBF_FIELDWIDTH;
  b[17] = 0;	/* decimal count */
  b += 32;

  /* terminator */
  b[0] = 0xd;
  ++b;		

  /*
   * records
   */
  for(i = 0; i < pm->numpolygons; ++i){
    b[0] = DCNIDS_DBF_RECORD_VALID_FLAG;
    ++b;
    /*
     * In the calculation of the totalsize we must ensure that the
     * last record has enough room for the terminating '\0'.
     */
    n = snprintf((char*)b, 2*DCNIDS_DBF_FIELDWIDTH + 1, "%3d%3d",
		 pm->polygons[i].code, pm->polygons[i].level);

    assert(n == 2*DCNIDS_DBF_FIELDWIDTH);

    /*
     * This will overwite the terminating '\0' of the previous record,
     * which is how it should be.
     */
    b += 2*DCNIDS_DBF_FIELDWIDTH;
  }
}

int dcnids_dbf_write(char *dbfname, struct dcnids_polygon_map_st *pm){

  int fd;
  int n;
  struct dcnids_dbf_st *dbf;
  int status = 0;

  dbf = dcnids_dbf_create(pm->numpolygons);
  if(dbf == NULL)
    return(-1);

  dcnids_dbf_insert_content(dbf, pm);

  fd = open(dbfname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd == -1){
    status = -1;
    goto End;
  }

  n = write(fd, dbf->b, dbf->size);
  if(n < 0)
    status = -1;

 End:
    dcnids_dbf_destroy(dbf);

    return(status);
}

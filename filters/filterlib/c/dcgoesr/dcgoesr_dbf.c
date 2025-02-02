/*
 * Copyright (c) 2010 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 * Copyright (c) 2025 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
/*
 * The code is a copy of the dcgini_dbf.c
 */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "err.h"
#include "dcgoesr_shp.h"	/* some of the insert_uintxx functions */
#include "dcgoesr.h"


struct dcgoesr_dbfinfo_st {
  int numfields;
  int fieldwidth;
  int numrecords;
};

struct dcgoesr_dbf_st {
  unsigned char *b;
  int size;
  struct dcgoesr_dbfinfo_st dbfinfo;
};

#define DCGOESR_DBF_HEADER_SIZE		32
#define DCGOESR_DBF_FIELD_DESC_SIZE	32
#define DCGOESR_DBF_TERMINATOR		'\015'
#define DCGOESR_DBF_FIELDNAME_LEN	10	/* excluding '\0' */

#define DCGOESR_DBF_FIELDWIDTH		3
#define DCGOESR_DBF_NUMFIELDS		1
#define DCGOESR_DBF_FIELDNAME		"level"

#define DCGOESR_DBF_RECORD_VALID_FLAG	0x20  /* blank => valid record */

static void dcgoesr_dbf_insert_uint16_little(unsigned char*b,
					    int pos, uint16_t u);
static int dcgoesr_dbf_total_size_bytes(struct dcgoesr_dbfinfo_st *dbfinfo);
static int dcgoesr_dbf_record_size_bytes(struct dcgoesr_dbfinfo_st *dbfinfo);
static int dcgoesr_dbf_header_size_bytes(struct dcgoesr_dbfinfo_st *dbfinfo);

static struct dcgoesr_dbf_st *dcgoesr_dbf_create(int numrecords);
static void dcgoesr_dbf_destroy(struct dcgoesr_dbf_st *dbf);
static void dcgoesr_dbf_insert_content(struct dcgoesr_dbf_st *dbf,
				      struct dcgoesr_point_map_st *pm);

static void dcgoesr_dbf_insert_uint16_little(unsigned char *p,
					    int pos, uint16_t u){

  unsigned char *b = p;

  b += pos;

  b[0] = u & 0xff;
  b[1] = (u >> 8) & 0xff;
}

static int dcgoesr_dbf_record_size_bytes(struct dcgoesr_dbfinfo_st *dbfinfo){

  int recordsize;

  /* include the deleted flag */
  recordsize = 1 + dbfinfo->fieldwidth * dbfinfo->numfields;	

  return(recordsize);
}

static int dcgoesr_dbf_header_size_bytes(struct dcgoesr_dbfinfo_st *dbfinfo){
  /*
   * This the "header length", which is everything up to the
   * start of the records:  header, fields descriptor array, terminator.
   */
  int headersize;

  /* includes 1 for the terminator after the fields descriptor array */ 
  headersize = DCGOESR_DBF_HEADER_SIZE +
    dbfinfo->numfields * DCGOESR_DBF_FIELD_DESC_SIZE + 1;
  
  return(headersize);
}

static int dcgoesr_dbf_total_size_bytes(struct dcgoesr_dbfinfo_st *dbfinfo){

  int recordsize, totalsize;

  recordsize = dcgoesr_dbf_record_size_bytes(dbfinfo);

  totalsize = dcgoesr_dbf_header_size_bytes(dbfinfo) +
    recordsize * dbfinfo->numrecords;

  /*
   * Add one to make easier for snprintf of the last record to insert
   * the terminating '\0'.
   */
  ++totalsize;

  return(totalsize);
}

static struct dcgoesr_dbf_st *dcgoesr_dbf_create(int numrecords){

  struct dcgoesr_dbf_st *dbf;
  int totalsize;

  dbf = malloc(sizeof(struct dcgoesr_dbf_st));
  if(dbf == NULL)
    return(NULL);

  dbf->dbfinfo.numfields = DCGOESR_DBF_NUMFIELDS;
  dbf->dbfinfo.fieldwidth = DCGOESR_DBF_FIELDWIDTH;
  dbf->dbfinfo.numrecords = numrecords;

  totalsize = dcgoesr_dbf_total_size_bytes(&dbf->dbfinfo);
  dbf->b = calloc(totalsize, sizeof(char));
  if(dbf->b == NULL){
    free(dbf);
    return(NULL);
  }
  dbf->size = totalsize;

  return(dbf);
}

static void dcgoesr_dbf_destroy(struct dcgoesr_dbf_st *dbf){

  if(dbf->b != NULL)
    free(dbf->b);

  free(dbf);
}

static void dcgoesr_dbf_insert_content(struct dcgoesr_dbf_st *dbf,
				      struct dcgoesr_point_map_st *pm){
  unsigned char *b;

  uint16_t recordsize;
  uint16_t headersize;
  size_t i;
  int n;
  
  recordsize = (uint16_t)dcgoesr_dbf_record_size_bytes(&dbf->dbfinfo);
  headersize = (uint16_t)dcgoesr_dbf_header_size_bytes(&dbf->dbfinfo);

  /* Header */
  dcgoesr_shp_insert_uint32_little(dbf->b, 4, 
				  (uint32_t)dbf->dbfinfo.numrecords);
  dcgoesr_dbf_insert_uint16_little(dbf->b, 8, headersize);
  dcgoesr_dbf_insert_uint16_little(dbf->b, 10, recordsize);

  /* Field descriptor array */
  b = &dbf->b[32];
  strncpy((char*)b, DCGOESR_DBF_FIELDNAME, DCGOESR_DBF_FIELDNAME_LEN);
  b[11] = 'N';
  b[16] = DCGOESR_DBF_FIELDWIDTH;
  b[17] = 0;	/* decimal count */
  b += 32;

  /* terminator */
  b[0] = 0xd;
  ++b;		

  /*
   * records
   */
  for(i = 0; i < pm->numpoints; ++i){
    b[0] = DCGOESR_DBF_RECORD_VALID_FLAG;
    ++b;
    /*
     * In the calculation of the totalsize we must ensure that the
     * last record has enough room for the terminating '\0'.
     */
    n = snprintf((char*)b, DCGOESR_DBF_FIELDWIDTH + 1, "%3d",
		 pm->points[i].level);

    assert(n == DCGOESR_DBF_FIELDWIDTH);

    /*
     * This will overwite the terminating '\0' of the previous record,
     * which is how it should be.
     */
    b += DCGOESR_DBF_FIELDWIDTH;
  }
}

/*
 * Public - declared in dcgoesr.h
 */
int dcgoesr_dbf_write(char *file, struct dcgoesr_point_map_st *pm){

  int fd;
  int n;
  struct dcgoesr_dbf_st *dbf;
  int status = 0;

  dbf = dcgoesr_dbf_create(pm->numpoints);
  if(dbf == NULL)
    return(-1);

  dcgoesr_dbf_insert_content(dbf, pm);

  fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd == -1){
    status = -1;
    goto End;
  }

  n = write(fd, dbf->b, dbf->size);
  if(n < 0)
    status = -1;

 End:
    dcgoesr_dbf_destroy(dbf);

    return(status);
}

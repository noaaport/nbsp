/*
 * Copyright (c) 2008 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <stdint.h>
#include "libspoolbdb/spoolbdb.h"

/*
 * memory based spool
 */
int nbsp_mspoolbdb_create(void);
int nbsp_mspoolbdb_destroy(void);
int nbsp_mspoolbdb_insert(char *fpath, void *fdata, size_t fdata_size);

/*
 * slot read functions
 */
int nbsp_mspoolbdb_open(char *fpath);
int nbsp_mspoolbdb_close(int sd);
size_t nbsp_mspoolbdb_read(int sd, void *data, size_t data_size);
size_t nbsp_mspoolbdb_datasize(int sd);
int nbsp_mspoolbdb_fpathsize(char *fpath, size_t *size);

/*
 * spool cache
 */
int nbsp_cspoolbdb_create(void);
int nbsp_cspoolbdb_destroy(void);
int nbsp_cspoolbdb_insert(char *fpath, void *fdata, size_t fdata_size);

/*
 * slot read functions
 */
int nbsp_cspoolbdb_open(char *fpath);
int nbsp_cspoolbdb_close(int sd);
size_t nbsp_cspoolbdb_read(int sd, void *data, size_t data_size);
size_t nbsp_cspoolbdb_datasize(int sd);
int nbsp_cspoolbdb_fpathsize(char *fpath, size_t *size);

/*
 * common
 */
void nbsp_mspoolbdb_dbstats(void);

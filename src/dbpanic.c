/*
 * Copyright (c) 2005 Jose F. Nieves <nieves@ltp.uprrp.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <db.h>
#include "globals.h"
#include "signal.h"
#include "dbpanic.h"

void nbsp_bdbpanic(void){

  int fd;

  if(get_dbpanic_flag() == 0)
    return;

  fd = open(g.mspoolbdb_panicfile, O_CREAT, g.mspoolbdb_panicfile_mode);
  if(fd != -1)
    (void)close(fd);
}

void nbsp_db_event_callback(DB_ENV *dbenv __attribute__ ((unused)),
			    uint32_t event,
			    void *event_info __attribute__ ((unused))){

  if(event == DB_EVENT_PANIC){
    set_quit_flag();
    set_dbpanic_flag();
  }
}

/*
 * Copyright (c) 2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "spooldb.h"

struct spooldb_st *spooldb_init(unsigned int numslots, int keysize,
				destroy_key_proc destroy_key){

  struct spooldb_st *spooldb = NULL;

  assert(numslots > 0);
  assert(keysize > 0);

  spooldb = malloc(sizeof(struct spooldb_st));
  if(spooldb == NULL)
    return(NULL);
  
  spooldb->keys = calloc(numslots, keysize);
  if(spooldb->keys == NULL){
    free(spooldb);
    return(NULL);
  }

  spooldb->numslots = numslots;
  spooldb->keysize = keysize;
  spooldb->slot = 0;
  spooldb->index = 0;
  spooldb->slotptr = &spooldb->keys[0];
  spooldb->destroy_key = destroy_key;

  return(spooldb);
}

void spooldb_destroy(struct spooldb_st *spooldb){

  free(spooldb->keys);
  free(spooldb);
}

int spooldb_insert(struct spooldb_st *spooldb, char *key){

  char *oldkey;
  int keysize;
  int status = 0;

  keysize = strlen(key) + 1;
  assert(keysize <= spooldb->keysize);
  
  oldkey = spooldb->slotptr;
  status = spooldb->destroy_key(oldkey);
  strncpy(oldkey, key, keysize);

  ++spooldb->slot;
  spooldb->index += spooldb->keysize;
  spooldb->slotptr += spooldb->keysize;

  if(spooldb->slot == spooldb->numslots){
    spooldb->slot = 0;
    spooldb->index = 0;
    spooldb->slotptr = &spooldb->keys[0];
  }

  return(status);
}

int spooldb_write(struct spooldb_st *spooldb, char *filename, mode_t mode){

  int status = 0;
  int fd = -1;
  ssize_t n, size;

  fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, mode);
  if(fd == -1)
    return(-1);

  size = sizeof(struct spooldb_st);
  n = write(fd, spooldb, size);
  if(n != size){
    status = -1;
    goto End;
  }

  size = spooldb->numslots * spooldb->keysize;
  n = write(fd, spooldb->keys, size);
  if(n != size){
    status = -1;
  }

 End:

  if(fd != -1)
    close(fd);

  return(status);
}

int spooldb_read(struct spooldb_st *spooldb, char *filename){
  /*
   * It is assumed that spooldb has already been initialized.
   * This function returns 0 if there are no errors, or
   *
   * -1 => reading error
   *  1 => the data in the file is inconsistent with the initialized spooldb.
   */  
  int status = 0;
  int fd = -1;
  ssize_t n, size;
  struct spooldb_st spooldb_saved;

  fd = open(filename, O_RDONLY);
  if(fd == -1)
    return(-1);
  
  size = sizeof(struct spooldb_st);
  n = read(fd, &spooldb_saved, size);
  if(n == -1){
    status = -1;
    goto End;
  }else if(n != size){
    status = 1;
    goto End;
  }

  /*
   * They keysize must be the same, and the new number of slots must
   * be equal or larger (but cannot be less) than the old one.
   * The "index" and "slot" are restored so the program starts
   * where it ended.
   */
  if((spooldb_saved.keysize != spooldb->keysize) ||
    (spooldb_saved.numslots > spooldb->numslots)){ 
    status = 1;
    goto End;
  }
  spooldb->slot = spooldb_saved.slot;
  spooldb->index = spooldb_saved.index;
  spooldb->slotptr = &spooldb->keys[spooldb->index];

  size = spooldb_saved.numslots * spooldb_saved.keysize;
  n = read(fd, spooldb->keys, size);
  if(n == -1){
    status = -1;
    goto End;
  }else if(n != size){
    status = 1;
    goto End;
  }

 End:

  if(fd != -1)
    close(fd);
  
  return(status);
}

unsigned int spooldb_get_slot(struct spooldb_st *spooldb){
  /*
   * Returns the index of the slot that will be occupied in the next
   * insert.
   */
  return(spooldb->slot);
}

/*
 * TEST
 */
#if 0
#include <stdio.h>
#include <err.h>
#include "spooldb.h"

#define NSLOTS  3
#define NKEYS	5
#define KEYSIZE	64

int destroy_key(char *key){

  if(key[0] != '\0')
    fprintf(stdout, "Deleted %s\n", key);
  else
    fprintf(stdout, "Nothing to delete.\n");

  return(0);
}

void print_report(struct spooldb_st *spooldb){

  unsigned int i;
  unsigned int index = 0;
  char *key;

  fprintf(stdout, "slot = %u\n", spooldb->slot);
  fprintf(stdout, "index = %u\n", spooldb->index);
  for(i = 0; i < spooldb->numslots; ++i){
    key = &spooldb->keys[index];
    if(key[0] != '\0')
      fprintf(stdout, "%u: %s\n", index, &spooldb->keys[index]);
    else
      fprintf(stdout, "%u: %s\n", index, "Empty");

    index += spooldb->keysize;
  }

  fprintf(stdout, "\n\n\n");
}

int main(void){

  int status = 0;
  struct spooldb_st *spooldb;
  char *keys[] = {"/var/noaaport/nbsp/spool/tjsj_cfwsju.txt",
		  "/var/noaaport/nbsp/spool/tjsj_faasju.txt",
		  "/var/noaaport/nbsp/spool/tjsj_zfpsju.txt",
		  "/var/noaaport/nbsp/spool/tjsj_fffsju.txt",
		  "/var/noaaport/nbsp/spool/tjsj_stpsju.txt"};
  int i;

  spooldb = spooldb_init(NSLOTS, KEYSIZE, destroy_key);
  if(spooldb == NULL)
    err(1, "Could not create spooldb.");

  for(i = 0; i < NKEYS; ++i){ 
    status = spooldb_insert(spooldb, keys[i]);
    if(status != 0)
      warn("Could not delete old key.");

    print_report(spooldb);
  }

  spooldb_destroy(spooldb);

  return(0);
}
#endif

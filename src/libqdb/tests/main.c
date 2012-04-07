#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include "qdb.h"

#define N	4
static nbsqtable_t *qtable = NULL;

static void *insert(void *arg){

  static int i = 0;
  int status = 0;
  int type;

  while(status == 0){
    ++i;
    type = i % N;
    status = nbspq_snd(qtable, (void*)&i, sizeof(int), type);
    if(status != 0)
      errx(1, "%s", db_strerror(qtable->dberror));

    sleep(1);
    /*
     * fprintf(stdout, "insert: nmax = %d\n", qlist.nmax); 
     */
  }

  return(NULL);
}

void extract(void){

  char line[2];
  int i;
  int status;

  while(fgets(line, 2, stdin) != NULL){
    
    status = nbspq_rcv_any(qtable, &i, sizeof(int), 500);
    if(status == 1){
      fprintf(stdout, "q empty\n");
    }else if(status == 0){
      fprintf(stdout, "%d\n", i);
    }else
      errx(1, "%s", db_strerror(qtable->dberror));

    fflush(stdout);

    if(status == 0)
      status = nbspq_stat(qtable);

    if(status == 0){
      for(i = 0; i <= qtable->n - 1; ++i)
	fprintf(stdout, "%d: %d\n", i, (qtable->q[i])->n);
    }
  }
}

int main(void){

  int status = 0;
  pthread_t t_id;
  pthread_attr_t attr;

  status = nbspq_open(&qtable, N, "q", sizeof(int), 100, 200);
  if(status != 0)
    errx(1, "%s", db_strerror(status));

  status = pthread_attr_init(&attr);
  if(status == 0)
       status = pthread_create(&t_id, &attr, insert, NULL);

  if(status != 0)
    err(1, "Cannot create insert thread.");

  extract();

  nbspq_close(qtable);

  return(0);
}

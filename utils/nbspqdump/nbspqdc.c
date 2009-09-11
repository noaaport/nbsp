/*
 * Copyright (c) 2005-2006 Jose F. Nieves <nieves@ltp.upr.clu.edu>
 *
 * See LICENSE
 *
 * $Id$
 *
 * Usage: nbspqdc -s <size> [-o <outputfile>]
 *
 * The program reads from stdin, and decodes each line, assumed to be of
 * the size specified via [-s].
 */ 
#include <stdio.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include "../../src/const.h"	/* FNAME_SIZE */
#include "../../src/packfpu.h"	/* packet_info_st, etc */

struct {
  char *opt_output_file;
  size_t opt_size;	/* size of data (dump is double) */
  /* variables */
  FILE *output_fp;
  unsigned char *dump;
  size_t dump_size;
  unsigned char *data;
  size_t data_size;
} g = {NULL, 0, NULL, NULL, 0, NULL, 0};

static void cleanup(void);
static int init(void);
static int process_data(void);
static void decode_record(void);
static int strto_uint(char *s, unsigned int *val);

int main(int argc, char **argv){

  int status = 0;
  int c;
  unsigned int size;
  char *optstr = "o:s:";
  char *usage = "nbspqdc [-o outputfile]  -s <size>";

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 's':
      status = strto_uint(optarg, &size);
      if(status != 0)
        errx(1, "Invalid value for [-s].");
      else{
        g.opt_size = (size_t)size;
      }
      break;
    case 'h':
    default:
      status = 1;
      errx(1, usage);
      break;
    }
  }

  if(optind < argc - 1)
     errx(1, "Too many arguments.");

  atexit(cleanup);

  if(status == 0)
    status = init();

  if(status == 0)
    status = process_data();

  return(status != 0 ? 1 : 0);
}

static void cleanup(void){

  if((g.output_fp != NULL) && (g.opt_output_file != NULL))
    fclose(g.output_fp);

  if(g.data != NULL)
    free(g.data);
}

static int init(void){

  if(g.opt_size == 0)
    errx(1, "No size specified.");

  /*
   * The allocated size does not include the byte for the '\n'.
   */
  g.data_size = g.opt_size;
  g.data = malloc(g.data_size);
  if(g.data == NULL){
    err(1, "Error from malloc()");
    return(-1);
  }

  g.dump_size = 2 * g.opt_size;
  g.dump = malloc(g.dump_size);
  if(g.dump == NULL){
    err(1, "Error from malloc()");
    return(-1);
  }

  return(0);
}

static int process_data(void){

  while(fread(g.dump, sizeof(char), g.dump_size, stdin) == g.dump_size){
    if(fgetc(stdin) != '\n'){
      errx(1, "Invalid value of the size option.");
      return(-1);
    }
    decode_record();
  }

  if(ferror(stdin)){
    err(1, "Error reading from stdin.");
    return(-1);
  }

  return(0);
}

static void decode_record(void){

  struct packet_info_st pinfo;
  unsigned char c;
  int i;

  for(i = 0; i < g.data_size; ++i){
    c = g.dump[2*i];
    if(isdigit(c))
      g.data[i] = (c - '0') << 4;
    else
      g.data[i] = ((c - 'a' + 10) << 4);

    c = g.dump[2*i + 1];
    if(isdigit(c))
      g.data[i] += (c - '0');
    else
      g.data[i] += (c - 'a' + 10);
  }

  pinfo.packet = (void*)g.data;
  pinfo.packet_size = g.data_size;
  if(nbsfp_unpack_fpath(&pinfo) != 0){
    warnx("Error from nbsfp_unpack_fpath().");
    return;
  }

  fprintf(stdout, "%u %d %d %d %d %s %s\n",
	  pinfo.seq_number,
	  pinfo.psh_product_type,
	  pinfo.psh_product_category,
	  pinfo.psh_product_code,
	  pinfo.np_channel_index,
	  pinfo.fname,
	  pinfo.fpath);
}

static int strto_uint(char *s, unsigned int *val){

  char *end;
  int status = 0;
  unsigned int v;
  int save_errno;
 
  save_errno = errno;
  errno = 0;

  v = strtoul(s, &end, 10);
  if((end == s) || (*end != '\0') || (errno != 0))
    status = 1;
  
  if(status == 0)
    *val = v;

  errno = save_errno;

  return(status);
}

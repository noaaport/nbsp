/*
 * $Id$
 */
/*
 * Usage: nbspinvrm [-t] [-s] [-v] [filename1 filename2 ...]
 *
 * The argument <filename> should be an "inventory" file. If
 * the argument is not given then the list is read from stdin.
 * Each line in an inventory file is the name or the full path of a file
 * to be deleted. Partial paths are not handled. The first line in the
 * inventory file must be a full path and all the files in an inventory
 * file must reside in the same directory. The tool will cd to the directory
 * referred to in the first entry and delete (unlink) all the files listed
 * in the inventory file. It will also delete the inventory file itself
 * if it was given in the argument.
 *
 * The [-v] option outputs the path of each file deleted or an error message
 * if there was an error with the file. The [-s] option suppresses writing
 * an error to stderr when a file could not be deleted. The [-t] flag is
 * just for testing.
 */
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <err.h>
#include <stdlib.h>
#include <libgen.h>
#include <errno.h>

#define FPATH_BUFFER_SIZE PATH_MAX

struct {
  int opt_test;		/* [-t] => only test */
  int opt_silent;	/* [-s] => don't write to stderr when removing a file*/
  int opt_verbose;	/* [-v] => output the name of each file removed */
  /* variables */
  FILE *fp;
  int cwdfd;		/* current dir fd for fchdir */
} g = {0, 0, 0, NULL, -1};

/* Functions */
static void cleanup(void);
static void process_file(char *input_file);
static void write_msg(char *msg);
static void write_err(char *s1, char *s2);
static void write_warnx(char *s1, char *s2);
static int e_unlink(char *fpath);

int main(int argc, char **argv){

  char *usage = "nbspinvrm -t -s -v [filename]";
  char *optstr = "stv";
  int status = 0;
  int c;
  int i;

  atexit(cleanup);

  while((status == 0) && ((c = getopt(argc, argv, optstr)) != -1)){
    switch(c){
    case 's':
      g.opt_silent = 1;
      break;
    case 't':
      g.opt_test = 1;
      break;
    case 'v':
      g.opt_verbose = 1;
      break;
    default:
      status = 1;
      errx(1, usage);
      break;
    }
  }

  if(optind == argc)
    errx(1, "Needs at least one file name argument.");

  g.cwdfd = open(".", O_RDONLY);
  if(g.cwdfd == -1)
    err(1, "Cannot open cwd.");

  for(i = optind; i < argc; ++i){
    process_file(argv[i]);
  }

  return(0);
}

/* Functions */
static void cleanup(void){

  if((g.fp != NULL && g.fp != stdin))
    (void)fclose(g.fp);

  if(g.cwdfd != -1)
    (void)close(g.cwdfd);
}

static void process_file(char *input_file){

  char fpath[FPATH_BUFFER_SIZE];
  char fpathc[FPATH_BUFFER_SIZE];	/* for the copy */
  char *dir = NULL;
  int size;
  int status = 0;

  g.fp = fopen(input_file, "r");
  if(g.fp == NULL){
    write_err("Cannot open", input_file);
    
    return;
  }

  while(fgets(fpath, FPATH_BUFFER_SIZE, g.fp) != NULL){
    size = strlen(fpath);
    if(fpath[size - 1] == '\n'){
      fpath[size - 1] = '\0';
      --size;
    }

    /*
     * Change to the directory indicated by the first path.
     * In FreeBSD dirname() returns a pointer to internal storage but
     * in Linux dirname() modifies the argument. This was causing
     * a bug in this program, now fixed. (Wed Oct 15 20:00:39 AST 2008)
     */

    if(dir == NULL){
      strncpy(fpathc, fpath, size + 1);
      dir = dirname(fpathc);
      status = chdir(dir);
      if(status != 0){
	write_err("Cannot chdir to", dir);
	/*
	 * If the directory does not exist, proceed as if we had deleted
	 * all the files. Otherwise return.
	 */
	if(errno == ENOENT)
	  break;
	else {
	  fclose(g.fp);
	  g.fp = NULL;
	  return;
	}
      }
    }
      
    if(g.opt_test == 0)
      status = e_unlink(fpath);

    if(status == 0)
      write_msg(fpath);
  }

  fclose(g.fp);
  g.fp = NULL;

  status = fchdir(g.cwdfd);
  if(status == 0){
    if(g.opt_test == 0){
      status = e_unlink(input_file);
    }
  }

  if(status == 0)
    write_msg(input_file);
}

static void write_msg(char *msg){

  if(g.opt_verbose == 1)
    fprintf(stdout, "%s\n", msg);
}

static void write_err(char *s1, char *s2){

  if(g.opt_verbose == 1)
    fprintf(stdout, "Error: %s %s %s\n", s1, s2, strerror(errno));
  
  if(g.opt_silent == 0)
    warn("%s %s.", s1, s2);
}

static void write_warnx(char *s1, char *s2){

  if(g.opt_verbose == 1)
    fprintf(stdout, "Warning: %s %s\n", s1, s2);
  
  if(g.opt_silent == 0)
    warnx("%s %s.", s1, s2);
}

static int e_unlink(char *fpath){

  int status;
  char *fbasename;

  /* The function assumes that we have chdir to the parent directory */
  if(fpath[0] == '/')
    fbasename = basename(fpath);
  else
    fbasename = fpath;

  status = unlink(fbasename);
  if(status != 0) {
    if(errno == ENOENT)
      write_warnx(fpath, "not found. Probably a retransmission duplicate.");
    else
      write_err("Cannot unlink", fpath);
  }

  return(status);
}

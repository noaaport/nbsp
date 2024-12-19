/*
 * This file is not part of the source. It is for reference purposes,
 * if we decide to add the option to process various files in one call,
 * either from stdin or the argument list, this is one way to do it.
 */

/* This would go after the init() */

  if(optind > argc - 1) {
    status = process_stdin();
  } else {
    while((optind <= argc - 1) && (status == 0)){
      g.opt_fpath = argv[optind++];
      ++g.opt_prod_seq_num;
      status = process_file();
    }
  }


int process_stdin(void)  {
  
  size_t fpath_size = 0;
  ssize_t fpath_len;
  int status = 0;

  while(status == 0) {
    fpath_len = getline(&g.stdin_fpath, &fpath_size, stdin);
    
    if(fpath_len == -1) {
      break;
    }
    
    if(g.stdin_fpath[fpath_len - 1] == '\n'){
      g.stdin_fpath[fpath_len - 1] = '\0';
      --fpath_len;
    }

    /* blank lines */
    if(fpath_len == 0)
      continue;

    g.opt_fpath = g.stdin_fpath;
    ++g.opt_prod_seq_num;

    status = process_file();
    if(status != 0) {
      log_err(0, "%s: %s", "Error processing", g.opt_fpath);
      status = 0;
    }
  }

  if(fpath_len == -1) {
    if(ferror(stdin) != 0)
      log_err(1, "%s", "Error from getline");
  }
  
  return(0);
}

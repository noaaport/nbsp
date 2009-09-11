BEGIN {
  first = 1;
  printf("%s", "-(");
}

END {
  printf("%s", ")");
}

!/^$/ && !/^#/ {
  if(first == 1){
    printf("%s", tolower($1));
  } else {
    printf("|%s", tolower($1));
  }
  first = 0;
}

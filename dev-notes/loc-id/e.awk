length($NF) == 2 {
  id = tolower($1);
  state = tolower($NF);
  printf("{\"%s\", \"%s\"},\n", id, state);
}

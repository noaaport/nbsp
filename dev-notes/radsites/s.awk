BEGIN {
  FS = "\t";
  OFS = "\t";
}

{
  s = substr($2, 1, 1) $1;
  print s;
}

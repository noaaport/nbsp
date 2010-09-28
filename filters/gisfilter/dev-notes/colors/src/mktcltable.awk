#
# cname must be passed via "-v"
#
BEGIN {
  n = 1;
}

{
  hex = substr($4,2);
  printf "set rgbcolor(%s,%d) {%s %s %s};  # %s\n", cname, n, $1, $2, $3, hex;
  ++n;
}

BEGIN {

  print "<h3>Products received</h3>";
  print "<table border>";
  printf("<tr><td>%s</td><td>%s</td><td>%s</td></tr>\n",
	 "time", "size", "name");
  fmt = "<tr><td>%s</td><td>%s</td><td>%s</td></tr>\n";
}

END {

  print "</table>";
}

{
  printf(fmt, $1, $6, $7);
}

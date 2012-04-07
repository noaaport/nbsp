BEGIN {

  time = 0;
  pctl_queue = 0;
  filter_queue = 0;
  server_queue = 0;

  fmt = "<tr>";
  for(i = 1; i <= 4; ++i){
    fmt = fmt "<td>%s</td>";
  }
  fmt = fmt "</tr>\n";

  print "<h3>Status of queues</h3>";
  print "<table border>";
  printf(fmt, "time", "processor", "filter", "server");
}

END {

  print "</table>";
}

{

  time = $1;

  pctl_queue = $2;
  filter_queue = $3;
  server_queue = $4;

  printf(fmt, time, pctl_queue, filter_queue, server_queue);
}


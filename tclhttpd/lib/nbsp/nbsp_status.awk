#
# $Id$
#
# Statistics by minute

BEGIN {

  time = 0;
  prod_tx = 0;
  prod_rx = 0;
  prod_rtx = 0;
  prod_missed = 0;
  frames_rx = 0;
  frames_jumps = 0;
  bytes_rx = 0;
  kbytes_rx_sec = 0;

  fmt = "<tr>";
  for(i = 1; i <= 9; ++i){
    fmt = fmt "<td>%s</td>";
  }
  fmt = fmt "</tr>\n";

  print "<h3>Minute Summaries for products, frames, bytes</h3>";
  print "<table border>";
  printf(fmt, "time", "products<br>transmitted", "products<br>received",
	 "products<br>retransmitted", "products<br>missed",
	 "frames<br>received", "frames<br>jumps",
	 "bytes<br>received", "kbytes/s");
}

END {

  print "</table>";
}

{

  time = $1;

  prod_tx = $7;
  prod_rx = $8;
  prod_rtx = $10;
  prod_missed = $9;
  frames_rx = $2;
  frames_jumps = $4;
  bytes_rx = $6;
  kbytes_rx_sec = int(bytes_rx/60000);  

  printf(fmt, time, prod_tx, prod_rx, prod_rtx, prod_missed,
	 frames_rx, frames_jumps, bytes_rx, kbytes_rx_sec);
}

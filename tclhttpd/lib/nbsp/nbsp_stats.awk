#
# $Id$
#
# Daily satistics summary

BEGIN {

  # These are the totals
  prod_transmitted = 0;
  prod_received = 0;
  prod_missed = 0;
  prod_rtx = 0;
  prod_recovered = 0;
  frames_received = 0;
  frames_jumps = 0;
  bytes_received = 0;

  fmt = "<tr><td>%s</td><td>%.3e</td><td>%.3e</td><td>%s</td></tr>\n";
  fmt1 = "<tr><td>%s</td><td>%f</td><td></td><td></td></tr>\n";
}

END {

  if(prod_transmitted != 0){
    prod_missed_fraction = prod_missed/prod_transmitted;
    prod_rtx_fraction = prod_rtx/prod_transmitted;
  }
  if(frames_received != 0){
    frames_jumps_fraction = frames_jumps/frames_received;
  }

  print "<h3>Daily Statistics for products, frames and bytes</h3>";
  print "<table border>";
  printf("<tr><td></td><td>%s</td><td>%s</td><td>%s</td></tr>\n",
	 "total", "per minute", "last minute");
  printf(fmt, "products trasmitted", prod_transmitted, prod_transmitted/NR,
	 prod_transmitted_last);
  printf(fmt, "products received", prod_received, prod_received/NR,
	 prod_received_last);
  printf(fmt, "products retransmitted", prod_rtx, prod_rtx/NR,
	 prod_rtx_last);
  printf(fmt, "products missed", prod_missed, prod_missed/NR,
	 prod_missed_last);
  printf(fmt, "frames received", frames_received, frames_received/NR,
	 frames_received_last);
  printf(fmt, "frames jumps", frames_jumps, frames_jumps/NR,
	 frames_jumps_last);
  printf(fmt, "bytes received", bytes_received, bytes_received/NR,
	 bytes_received_last);
  printf(fmt1, "prod missing fraction", prod_missed_fraction);
  print "</table>";
}

{
  prod_transmitted_last = $7;
  prod_received_last = $8;
  prod_missed_last = $9;
  prod_rtx_last = $10;
  prod_recovered_last = $13;
  frames_received_last = $2;
  frames_jumps_last = $4;
  bytes_received_last = $6;

  prod_transmitted += $7;
  prod_received += $8;
  prod_missed += $9;
  prod_rtx += $10;
  prod_recovered += $13;
  frames_received += $2;
  frames_jumps += $4;
  bytes_received += $6;
}

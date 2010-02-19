BEGIN {
#  FS = "\t";
  OFS = "\t";
}

NR >= 8 {
  type = $NF;
  timeoffset = $(NF - 1);
  alt = $(NF - 2);
  lon = $(NF - 3);
  lat = $(NF - 4);
 
  print $2, lat, lon;
}

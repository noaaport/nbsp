END {
  lon2 = lon1 + cellsize * nx;
  lat2 = lat1 + cellsize * ny;
  printf("%s\n", name);
  printf("    EXTENT %f %f %f %f\n", lon1, lat1, lon2, lat2);
  printf("    SIZE %d %d\n", nx, ny);
}

/^ncols/{
  nx = $2;
}

/^nrows/{
  ny = $2;
}

/^xllcorner/{
  lon1 = $2;
}

/^yllcorner/{
  lat1 = $2;
}

/^cellsize/{
  cellsize = $2;
}

NR >= 7{
  exit;
}

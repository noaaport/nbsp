BEGIN {
  FS = ";";
  OFS = ";";
}

{
  icao = $1;
  block_number = $2;
  station_number = $3;
  place_name = $4;
  state = $5;
  country = $6;
  wmo_region = $7;
  lat = $8;
  lon = $9;
  
  if(state != ""){
    printf("set metarfilter(icao,%s) \"%s%, %s, %s, %s\"\n",
	   tolower(icao), lat, lon, place_name, state);
  }
}

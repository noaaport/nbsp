# README #

### Sat Jan  4 17:45:23 AST 2025 ###

The strategy is:

1) nc2csv_xycmi - extracts the "raw" x,y,cmi values from the nc file,
   constructs the corresponding physical xrad,yrad.cmi values using the
   scale and offset parameters of the nc file, and writes the xrad,yrad,cmi
   trio to stdout in cvs format.
   
2) xy2lonlat - inputs the output of (1) and converts each xrad,yrad pair
   to the corresponding lonrad,latrad and re-emits the lonrad,latrad,cmi
   trio (in csv format) to stdout.

These steps are incorporated in the main "nc2csv_lonlatcmi" script

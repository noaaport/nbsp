#!/usr/local/bin/tclsh8.6

source "radstations.tcl";

set g [subst {
  id       rad_n0r_south
  maptmpl  map_rad.tmpl
  options  {extent {[nbsp::radstations::extent_bystate ar la nm ok tx]}\
			size {800 600} awips1 n0r}
  outputfile img/rad/n0rsouth.png
  inputpatt  *.tif
  inputdirs  {[nbsp::radstations::inputdirs_bystate \
		   "rad/tif/%{sss}/n0r" ar la nm ok tx]}
}];

puts $g

set r "-n0r([join [nbsp::radstations::bystate ar la nm ok tx] "|"])";
puts $r;


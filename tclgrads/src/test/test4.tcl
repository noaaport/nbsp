#!/usr/local/bin/tclsh8.6

#lappend auto_path "/usr/local/lib/nbsp/tcl/grads";
lappend auto_path "../";
package require grads;

#
# main
#
set ctlfile "yq.ctl";

grads::init;
grads::open $ctlfile;

# GrADS NOTE: N.B. Coordinate transform queries are only valid after
# something has been displayed, and the transformations apply only
# to the most recent item that has been displayed. 
grads::exec d tmpprs;

set lon -125;
set lat 37;

puts "Transforming $lon $lat";

grads::transform w2gr $lon $lat gx gy;
puts "$gx $gy";

grads::transform gr2w $gx $gy lon lat;
puts "$lon $lat";

grads::transform w2xy $lon $lat x y;
puts "$x $y";

grads::transform xy2w $x $y lon lat;
puts "$lon $lat";

grads::transform xy2gr $x $y gx gy;
puts "$gx $gy";

grads::transform gr2xy $gx $gy x y;
puts "$x $y";

grads::end;

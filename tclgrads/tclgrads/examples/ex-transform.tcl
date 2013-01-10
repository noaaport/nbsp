#!%TCLSH%
#
# $Id$
#
package require grads;

grads::init;
grads::open "yq";

# Set something and transform them back and forth
grads::exec d tmpprs;  # just to establish a reference
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

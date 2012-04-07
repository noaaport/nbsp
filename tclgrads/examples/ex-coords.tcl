#!%TCLSH%
#
# $Id$
#
package require grads;

grads::init;
grads::open "yq";

# All the latitudes
gradsu::coords "lat" lat; 
foreach l $lat {
    puts $l;
}

# Set a range and print only that range
puts "-"
grads::exec set y 1 10;
gradsu::coords "lat" lat -r;
foreach l $lat {
    puts $l;
}

puts "-";
grads::exec set x 20 30;
gradsu::coords "lon" lon -r;
foreach l $lon {
    puts $l;
}

# Without the "-r" they should retrieve and print all the values
puts "-";
gradsu::coords "lat" lat;
foreach l $lat {
    puts $l;
}

puts "-";
gradsu::coords "lon" lon;
foreach l $lon {
    puts $l;
}

grads::end;

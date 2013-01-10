#!%TCLSH%
#
# $Id$
#
package require grads;

grads::init;
grads::open "yq";

# All longitudes
grads::get_lons lon;
foreach l $lon {
    puts $l;
}

# One longitude
puts "-"
::grads::exec set x 10;
grads::get_lons lon -r;
foreach l $lon {
    puts $l;
}

# A range
puts "-"
grads::exec set x 1 10;
grads::get_lons lon -r;
foreach l $lon {
    puts $l;
}

# This should again print them all
grads::get_lons lon;
foreach l $lon {
    puts $l;
}

grads::end;

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

grads::get_dimensions a;

grads::get_lons lon;
foreach l $lon {
    puts $l;
}

puts "-"
::grads::exec set x 10;
grads::get_lons lon -r;
foreach l $lon {
    puts $l;
}

puts "-"
grads::exec set x 1 10;
grads::get_lons lon -r;
foreach l $lon {
    puts $l;
}

grads::end;

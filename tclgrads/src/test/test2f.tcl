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

gradsu::coords "lat" lat; 
foreach l $lat {
    puts $l;
}

puts "-"
grads::exec set y 1 10;
gradsu::coords "lat" lat -r;
foreach l $lat {
    puts $l;
}

puts "-"
grads::exec set x 20 30;
gradsu::coords "lon" lon -r;
foreach l $lon {
    puts $l;
}

grads::end;

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

grads::get_lons l;
foreach v $l {
    puts $v;
}

grads::get_lats l;
foreach v $l {
    puts $v;
}

gradsu::lons l;
foreach v $l {
    puts $v;
}

gradsu::lats l;
foreach v $l {
    puts $v;
}

grads::end;

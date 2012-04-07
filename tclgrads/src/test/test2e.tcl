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

grads::get_lats lat;
foreach l $lat {
    puts $l;
}

puts "-"
::grads::exec set y 10;
grads::get_lats lat -r;
foreach l $lat {
    puts $l;
}

puts "-"
grads::exec set y 1 10;
grads::get_lats lat -r;
foreach l $lat {
    puts $l;
}

grads::end;

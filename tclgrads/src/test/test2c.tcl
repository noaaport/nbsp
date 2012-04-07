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

grads::get_levels levels;
foreach l $levels {
    puts $l;
}

puts "-"
grads::get_levels levels -r;
foreach l $levels {
    puts $l;
}

puts "-"
grads::exec set z 1 10;
grads::get_levels levels -r;
foreach l $levels {
    puts $l;
}

grads::end;

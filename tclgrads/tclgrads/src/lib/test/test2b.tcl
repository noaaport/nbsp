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
foreach k [lsort [array names a]] {
    puts "a($k) = $a($k)";
}

grads::get_levels levels;
foreach z $levels {
    puts $z;
}

gradsu::levels levels;
foreach z $levels {
    puts $z;
}

grads::get_dimensions a;
foreach k [lsort [array names a]] {
    puts "a($k) = $a($k)";
}

grads::end;

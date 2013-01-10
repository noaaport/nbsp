#!/usr/local/bin/tclsh8.6

#lappend auto_path "/usr/local/lib/nbsp/tcl/grads";
lappend auto_path "../";
package require grads;
#source ../grads.tcl

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

grads::get_times times;
foreach t $times {
    puts $t;
}

grads::get_dimensions a;
foreach k [lsort [array names a]] {
    puts "a($k) = $a($k)";
}

gradsu::mset x 10 y 10 t "1 3";
gradsu::getval1 tmpprs t tmpprs;
foreach t $tmpprs {
    puts $t;
}

set tmpprs [grads::eval_expr1 tmpprs t];
foreach t $tmpprs {
    puts $t;
}

grads::end;

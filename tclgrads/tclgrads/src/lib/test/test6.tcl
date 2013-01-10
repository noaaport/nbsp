#!/usr/local/bin/tclsh8.6

#lappend auto_path "/usr/local/lib/nbsp/tcl/grads";
lappend auto_path "../";
#package require grads;
source ../grads.tcl

#
# main
#
set ctlfile "yq.ctl";

grads::init;
grads::open $ctlfile;

grads::exec set x 10 14;
grads::exec set y 11 15;

set m [grads::eval_expr_xy "lon|lat|tmpprs" -r];
puts [$m columns];
puts [$m rows];
set i 0;
while {$i < [$m rows]} {
    set row [$m get row $i];
    foreach j $row {
	puts -nonewline "$j ";
    }
    puts "";
    incr i;
}

set m [grads::eval_expr_xy "lon,lat,tmpprs" -r -t -s "," -S ":"];
set i 0;
while {$i < [$m rows]} {
    set row [$m get row $i];
    foreach j $row {
	puts -nonewline "$j ";
    }
    puts "";
    incr i;
}

grads::end;

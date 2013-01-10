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

grads::exec set x 10 14;
grads::exec set y 11 15;

gradsu::getval2 "lon|lat|tmpprs" x y m -r;
set i 1;
set nrows [llength $m];
while {$i <= $nrows} {
    set row [lindex $m $i];
    set j 1;
    set n [llength $row];
    while {$j <= $n} {
	puts -nonewline "[lindex $row $j] ";
	incr j;
    }
    puts "";
    incr i;
}

gradsu::getval2 "lon,lat,tmpprs" x y m -r -s "," -S ":";
set i 1;
set nrows [llength $m];
while {$i <= $nrows} {
    set row [lindex $m $i];
    set j 1;
    set n [llength $row];
    while {$j <= $n} {
	puts -nonewline "[lindex $row $j] ";
	incr j;
    }
    puts "";
    incr i;
}

grads::end;

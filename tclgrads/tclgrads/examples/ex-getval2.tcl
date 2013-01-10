#!%TCLSH%
#
# $Id$
#
package require grads;

# This example is similar to "ex-eval_expr_xy", but uses the
# gradsu::getval2 function, which has a slightly different convention
# from grads::eval_expr_xy.

grads::init;
grads::open "yq";

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

# Same thing with separating characters n the input <expression>
# and for the ourput result.

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

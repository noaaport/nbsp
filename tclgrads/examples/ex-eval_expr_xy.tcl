#!%TCLSH%
#
# $Id$
#
package require grads;

grads::init;
grads::open "yq";;

# Set the range in the xy plane
grads::exec set x 10 14;
grads::exec set y 11 17;

## Evaluate the "tmpprs" variable at each point and print the matrix.
## The first commented line will store in the matrix the lon/lat coordinates
## along with the value of the variable at each point.
## The second commented line will store in the matrix the xy wind components
## along with the tmpprs at each point. 

grads::eval_expr_xy "tmpprs" m -r;
# grads::eval_expr_xy "lat|lon|tmpprs" m -r -T;
# grads::eval_expr_xy "tmpprs|ugrdprs|vgrdprs" m -r;

## $m is now a matrix object that can be manipulated with the
## functions from the struct::matrix package of the tcllib.

puts "columns = [$m columns]";
puts "rows = [$m rows]";

set i 0;
while {$i < [$m rows]} {
    set row [$m get row $i];
    puts [join $row];
    incr i;
}

grads::end;

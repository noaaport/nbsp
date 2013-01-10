#!%TCLSH%
#
# $Id$
#
package require grads;

grads::init;
grads::open "yq";

gradsu::mset x 10 y 10 t "1 3";

# Here $t contains only the values
grads::eval_expr1 tmpprs t tmpprs;
foreach t $tmpprs {
    puts $t;
}

# Here the first element of $t is the number of items
gradsu::getval1 tmpprs t tmpprs;
foreach t $tmpprs {
    puts $t;
}

grads::end;

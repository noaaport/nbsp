#!%TCLSH%
#
# $Id$
#
package require grads;

grads::init;
grads::open "yq";
grads::get_vars vars;
grads::end;

foreach v $vars {
    puts $v;
}


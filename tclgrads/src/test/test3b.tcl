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

#grads::exec set gxout shaded;
#grads::exec d tmpprs;
gradsu::mset gxout shaded;
gradsu::display tmpprs;
#grads::exec printim test3b.png;
gradsu::printim test3b.png;

grads::end;

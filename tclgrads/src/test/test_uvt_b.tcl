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

gradsu::times times;
set t 1;
foreach date $times {
    gradsu::mset t $t;
    gradsu::display "skip(ugrdprs,3);skip(vgrdprs,3);tmpprs";
    gradsu::draw "title Wind/Temp $date";
    gradsu::printim uvt-$date.png;
    gradsu::clear;
    incr t;
}

grads::end;

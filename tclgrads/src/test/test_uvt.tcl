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

grads::get_times times;
set t 1;
foreach date $times {
    grads::exec set t $t;
    grads::exec d "skip(ugrdprs,3);skip(vgrdprs,3);tmpprs";
    grads::exec draw title Wind/Temp $date;
    grads::exec printim uvt-$date.png;
    grads::exec clear;
    incr t;
}

grads::end;

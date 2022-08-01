#!/usr/local/bin/tclsh8.6

package require Tclx;

set nbspinfifo "infeed.fifo";
append nbspinlock $nbspinfifo ".lock";

set LOCK [open $nbspinlock {WRONLY CREAT}]
flock -write $LOCK;

set FIFO [open $nbspinfifo {WRONLY}]
puts $FIFO "OK"
close $FIFO

close $LOCK
file delete $nbspinlock


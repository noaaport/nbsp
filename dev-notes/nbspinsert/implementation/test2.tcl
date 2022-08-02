#!/usr/bin/tclsh

package require Tclx;
source "f.tcl";

set nbspinfifo "infeed.fifo";
set nbspinlock "infeed.lock";

set FIFO [open $nbspinfifo {WRONLY}]
open_lock

infeed_lock
puts $FIFO "OK"
infeed_unlock

close $LOCK

# in case the reader has exited
catch {close $FIFO}




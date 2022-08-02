#!/usr/bin/tclsh

package require Tclx;
source "f.tcl";

set nbspinfifo "infeed.fifo";
set nbspinlock "infeed.lock";

set FIFO [open $nbspinfifo {WRONLY}]
open_lock

infeed_lock;
exec sleep 10
puts $FIFO "working"
infeed_unlock;

close_lock

# in case the reader has exited
catch {close $FIFO}

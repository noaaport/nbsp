#!/usr/local/bin/tclsh8.6
#
# Proof thet fcntl() in tclx does not work on fifos in freebsd. Similar
# to a.c
#
package require Tclx;

set nbspinfifo "infeed.fifo";

set FIFO [open $nbspinfifo {WRONLY}]
flock -write $FIFO;

puts $FIFO "OK"
close $FIFO

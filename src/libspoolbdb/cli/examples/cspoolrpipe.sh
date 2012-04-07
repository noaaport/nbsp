#!/bin/sh

./cspoolrpipe.tcl -p "nbsppipe -d /tmp -o $1 -g %s" \
    -f  /var/noaaport/nbsp/spool/tjsj/tjsj_srpu52-rr5sju.200059_86445675 $1

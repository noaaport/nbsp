#!/usr/bin/tclsh

source radstations.tcl

# proc nbsp::radstations::extent_bysite {site {shift 2}}
# proc nbsp::radstations::extent_bysitelist {args}
# proc nbsp::radstations::extent_bystate {args}

set a [list ama bro crp dfx dyx ewx]
set b [list fws grk hgx lbb maf sjt]

set A [::nbsp::radstations::extent_bysitelist $a $b];
set B [::nbsp::radstations::extent_bystate tx];

puts $A
puts $B


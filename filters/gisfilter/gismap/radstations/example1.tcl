#!/usr/bin/tclsh

source "radstations.tcl"

#puts [join [nbsp::radstations::bystate ar la nm ok tx] "|"];
puts [join [nbsp::radstations::bystate ok tx] "|"];

# proc nbsp::radstations::extent_bystate {args}
# proc nbsp::radstations::sitelist {}


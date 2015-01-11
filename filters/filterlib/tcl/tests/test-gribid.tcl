#!/usr/bin/tclsh

set common(localconfdirs) [list];

source "filters-gribid.def";
source "filters-gribid.lib";

set station [lindex $argv 0];
set wmoid [lindex $argv 1];

set gridid "0";

# First check if it is an ncep
set _g [get_gridid_from_ncep_kwbx $station $wmoid];

if {${_g} eq "0"} {
    set _g [get_gridid_from_wmoid $wmoid];
}

if {${_g} ne "0"} {
    set gridid ${_g};
}
unset _g;

puts $gridid;

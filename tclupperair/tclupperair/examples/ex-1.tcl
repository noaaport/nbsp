#!/usr/local/bin/tclsh8.4

source "../src/fm35.tcl";

set body {USDL02 EDZW 151800 TTAA 15171 10393 99007 01106 25005 00170 01508 25008 92788 04903 30014 85449 08310 31513 70930 15710 33515 50540 30722 34523}

::upperair::fm35::decode $body;

set r "";
foreach level [::upperair::fm35::get_levels] {
    append r "$level:";
    append r [join [::upperair::fm35::get_data $level] " "];
    append r "|";
}
puts [string trimright $r "|"];

#!/usr/local/bin/tclsh8.4

package require Expect;
package require gempak::sflist;
#source sflist.tcl;

source "/etc/local/nbsp/gempak.env";

# Find the current file in the surface data directory
set surfacedir [file join $env(GEMDATA) "surface"];
set now [expr [clock seconds]];
set last_hour [clock format [expr $now - 3600] -format "%H" -gmt true];

set sffile_name [clock format $now  -format "%Y%m%d" -gmt true];
append sffile_name "_sao.gem";
set sffile [file join $surfacedir $sffile_name];

::gempak::sflist::sffile $sffile;
::gempak::sflist::dattim "all";
::gempak::sflist::sfparm tmpf relh dwpf pmsl sped drct;
::gempak::sflist::stations tjsj
## ::gempak::sflist::sfparm slat slon tmpf relh dwpf pmsl sped drct;

::gempak::sflist::init;
::gempak::sflist::run;
::gempak::sflist::end;

puts [::gempak::sflist::output];

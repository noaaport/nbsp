#!/usr/local/bin/tclsh8.4

package require Expect;
package require gempak::gdinfo;

source "/etc/local/nbsp/gempak.env";

# Choose a model and file. We will pick the latest gfs211.
set datadir [file join $env(GEMDATA) "model" "gfs"];
set filelist [lsort [glob -directory $datadir "*gfs211.gem"]];
set gdfile [lindex $filelist end];

::gempak::gdinfo::define gdfile $gdfile;
::gempak::gdinfo::define lstall "yes";
::gempak::gdinfo::define output "t";
::gempak::gdinfo::define gdattim "all";
::gempak::gdinfo::define glevel "all";
::gempak::gdinfo::define gvcord "all";
::gempak::gdinfo::define gfunc "all";

::gempak::gdinfo::init;
::gempak::gdinfo::run;
::gempak::gdinfo::end;

puts [::gempak::gdinfo::output];

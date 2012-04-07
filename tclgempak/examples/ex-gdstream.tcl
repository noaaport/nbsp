#!%TCLSH%
#
# $Id$
#
package require gempak;

# Uncomment this if necessary
source /usr/local/etc/nbsp/gempak.env

# Choose a model and file. We will pick the latest gfs211.
set datadir [file join $env(GEMDATA) "model" "gfs"];
set filelist [lsort [glob -directory $datadir "*gfs211.gem"]];
set gdfile [lindex $filelist end];

# main

gempak::define gdfile $gdfile

gempak::define garea "tx";

gempak::define gvect "wnd"
gempak::define glevel 900;
gempak::define gvcord "pres";

gempak::define line "3//1";
gempak::define map "1/7";
gempak::define title "5";
gempak::define latlon "2/10/1/1/5;5";

# Do several forecast times
foreach t [list 24 36 48 72] {
    gempak::define gdattim f$t;
    gempak::define device "gif|gdstream_$t.gif";
    gempak::init gdstream;
    gempak::run;
    gempak::end;
}

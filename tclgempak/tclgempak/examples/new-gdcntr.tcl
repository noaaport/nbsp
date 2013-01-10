#!/usr/local/bin/tclsh8.4
#
# $Id$
#
package require gempak;

# Uncomment this if necessary
source /usr/local/etc/nbsp/gempak.env

# Choose a model and file
set datadir [file join $env(GEMDATA) "model" "gfs"];
set gdfile_name "2009020818_gfs211.gem";

set gdfile [file join $datadir $gdfile_name];

# main

gempak::define gdfile $gdfile
gempak::define device "gif|gdcntr.gif";

gempak::garea_lat1 15;
gempak::garea_lon1 -125
gempak::garea_lat2 50
gempak::garea_lon2 -60;
gempak::set_garea;

gempak::define gdattim "last"

gempak::define gdpfun "tmpf"
gempak::define glevel 900;
gempak::define gvcord "pres";

# we will do both
gempak::define type "c/f";

# Put more contours than the defaults
gempak::cint_interval 3;
gempak::set_cint;

gempak::init gdplot2;
gempak::run;
gempak::end;

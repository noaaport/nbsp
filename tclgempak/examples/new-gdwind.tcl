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
gempak::define device "gif|gdwind.gif";

gempak::garea_lat1 15;
gempak::garea_lon1 -125
gempak::garea_lat2 50
gempak::garea_lon2 -60;
gempak::set_garea;

gempak::define gdattim "last"

gempak::define gdpfun "wnd";
gempak::define type "a"
gempak::define glevel "1000";
gempak::define gvcord "pres";

gempak::wind_symbol_type "a";
gempak::wind_symbol_units "k";
gempak::wind_symbol_color "4";
gempak::set_wind;

gempak::refvec_magnitude 10;
gempak::set_refvec;

gempak::init gdplot2;
gempak::run;
gempak::end;

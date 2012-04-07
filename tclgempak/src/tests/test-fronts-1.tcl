#!/usr/local/bin/tclsh8.4
#
# $Id$
#
# Test gempak::garea_xxx functions
#

# To test without installing uncomment the nextline
lappend auto_path "..";
package require gempak;

set datafile "/var/noaaport/data/digatmos/nwx/hpc/fronts/latest.front";

gempak::init gpfront;

gempak::define asfil $datafile;
gempak::define device "gif|fronts.gif";

gempak::garea_lat1 15;
gempak::garea_lon1 -125;
gempak::garea_lat2 50;
gempak::garea_lon2 -60;
gempak::set_garea;

gempak::run;
gempak::end;

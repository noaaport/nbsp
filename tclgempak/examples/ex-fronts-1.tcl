#!%TCLSH%
#
# $Id$
#
package require gempak;

# Uncomment this if necessary
source /usr/local/etc/nbsp/gempak.env

# Edit this if necessary
set datafile "/var/noaaport/data/digatmos/nwx/hpc/fronts/latest";

# main
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

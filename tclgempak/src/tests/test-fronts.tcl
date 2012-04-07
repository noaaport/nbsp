#!/usr/local/bin/tclsh8.4
#
# $Id$
#

# To test without installing uncomment the nextline
lappend auto_path "..";
package require gempak;

set datafile "/var/noaaport/data/digatmos/nwx/hpc/fronts/latest.front";

gempak::init gpfront;

gempak::define asfil $datafile;
gempak::define device "gif|fronts.gif";
gempak::define garea "us";

gempak::run;
gempak::end;

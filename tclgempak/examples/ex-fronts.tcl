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
gempak::define garea "us";

gempak::run;
gempak::end;

#!%TCLSH%
#
# $Id$
#
package require gempak;

# Uncomment this if necessary
source /usr/local/etc/nbsp/gempak.env

# Choose a model and file
set datadir [file join $env(GEMDATA) "model" "gfs"];
set gdfile_name "2009011718_gfs211.gem";

set gdfile [file join $datadir $gdfile_name];

# main

gempak::define gdfile $gdfile
gempak::define device "gif|gdcntr.gif";

gempak::define garea "tx";

# 24 hour forecast
gempak::define gdattim "f24";

gempak::define gfunc "tmpf"
gempak::define glevel 900;
gempak::define gvcord "pres";

# for contour only or fill only use one of these
## gempak::ctype_fill;
## gempak::ctype_contour;

# we will do both
gempak::ctype_fc;

# Put more contours than the defaults
gempak::cint_interval 3;
gempak::set_cint;

gempak::init gdcntr;
gempak::run;
gempak::end;

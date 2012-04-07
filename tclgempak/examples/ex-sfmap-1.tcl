#!%TCLSH%
#
# $Id$
#
package require gempak;

# Uncomment this if necessary
source /usr/local/etc/nbsp/gempak.env

# Find the current file in the surface data directory
set surfacedir [file join $env(GEMDATA) "surface"];
set now [expr [clock seconds]];
set last_hour [clock format [expr $now - 3600] -format "%H" -gmt true];

set sffile_name [clock format $now  -format "%Y%m%d" -gmt true];
append sffile_name "_sao.gem";
set sffile [file join $surfacedir $sffile_name];

# main

gempak::define sffile $sffile
gempak::define device "gif|sfmap.gif";

gempak::define garea "15;-125;50;-60";
gempak::define area "garea";
gempak::define dattim "/$last_hour";

#
# Now set the equivalent of
#
# gempak::define sfparm "skyc:0.75;tmpf:0.75;brbk:0.75";
# gempak::define colors "26;2;26";
#
 
gempak::sfparm_name "skyc";
gempak::sfparm_size 0.75;
gempak::set_sfparm;
gempak::colors 26;

gempak::sfparm_name "tmpf";
gempak::sfparm_size 0.75;
gempak::append_sfparm;
gempak::append_colors 2;

gempak::sfparm_name "brbk";
gempak::sfparm_size 0.75;
gempak::append_sfparm;
gempak::append_colors "26";

gempak::init sfmap;
gempak::run;
gempak::end;

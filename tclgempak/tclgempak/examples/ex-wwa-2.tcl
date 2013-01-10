#!%TCLSH%
#
# $Id$
#
package require gempak;

# Uncomment this if necessary
source /usr/local/etc/nbsp/gempak.env

# main
gempak::init gpmap;

gempak::define device "gif|wwa.gif";

::gempak::garea_lat1 15;
::gempak::garea_lon1 -125
::gempak::garea_lat2 50
::gempak::garea_lon2 -60;
::gempak::set_garea;

::gempak::latlon_color 1;
::gempak::latlon_linetype 10;
::gempak::latlon_linewidth 1;
::gempak::latlon_freq 15 15;
::gempak::set_latlon;

::gempak::wstm_end_time {last};
::gempak::wstm_outline_flag {yes};
::gempak::wstm_outline_width_warn {1};
::gempak::wstm_outline_width_watch {1};
::gempak::wstm_outline_width_advisory {1};
::gempak::set_wstm;

gempak::run;
gempak::end;

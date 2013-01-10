#!/usr/local/bin/tclsh8.4

lappend auto_path "..";
package require gempak;

gempak::init gpmap;

gempak::define map 1;
gempak::define proj "str/90;-80;0";
gempak::define garea "-10;-130;-10;50";

#gempak::define latlon "2/10/1/1/10;10";
gempak::latlon_color 2;
gempak::latlon_linetype 10;
gempak::latlon_linewidth 1;
gempak::latlon_freq 1 1;
gempak::latlon_incr 10 10;
gempak::set_latlon;

gempak::device_device "gif";
gempak::device_name "latlon.gif";
gempak::set_device;

gempak::run;
gempak::end;

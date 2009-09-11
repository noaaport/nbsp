#!/usr/local/bin/tclsh8.4
#
# $Id$
#
set gempak_datadir	"/var/noaaport/data/gempak"
set gempak_satdir	"images/sat"
set gempak_sattmpdir	"tmp"

set nbspsat_tool	"/usr/local/bin/nbspsat"
##set nbspsat_tool	"/var/noaaport/nbsp/filters/nbspsat"

set nbspsat_opts	"-b -g -d $gempak_sattmpdir"

set tig_sector(0)	"NHEM-COMP"
set tig_sector(1)	"EAST-CONUS"
set tig_sector(2)	"WEST-CONUS"
set tig_sector(3)	"AK-REGIONAL"
set tig_sector(4)	"AK-NATIONAL"
set tig_sector(5)	"HI-REGIONAL"
set tig_sector(6)	"HI-NATIONAL"
set tig_sector(7)	"PR-REGIONAL"
set tig_sector(8)	"PR-NATIONAL"
set tig_sector(9)	"SUPER-NATIONAL"
set tig_sector(10)	"NHEM-MULTICOMP"

set tig_channel(1)	"VIS"
set tig_channel(2)	"3.9"
set tig_channel(3)	"WV"
set tig_channel(4)	"IR"
set tig_channel(5)	"12.0"
set tig_channel(6)	"13.3"
set tig_channel(7)	"1.3"
set tig_channel(16)	"LI"
set tig_channel(17)	"PW"
set tig_channel(18)	"SFC-T"
set tig_channel(19)	"CAPE"
set tig_channel(27)	"CTP"
set tig_channel(41)	"SOUND-14.37"
set tig_channel(41)	"SOUND-14.71"
set tig_channel(42)	"SOUND-14.37"
set tig_channel(43)	"SOUND-14.06"
set tig_channel(44)	"SOUND-13.64"
set tig_channel(45)	"SOUND-13.37"
set tig_channel(46)	"SOUND-12.66"
set tig_channel(47)	"SOUND-12.02"
set tig_channel(48)	"SOUND-11.03"
set tig_channel(49)	"SOUND-9.71"
set tig_channel(50)	"SOUND-7.43"
set tig_channel(51)	"SOUND-7.02"
set tig_channel(52)	"SOUND-6.51"
set tig_channel(53)	"SOUND-4.57"
set tig_channel(54)	"SOUND-4.52"
set tig_channel(55)	"SOUND-4.45"
set tig_channel(56)	"SOUND-4.13"
set tig_channel(57)	"SOUND-3.98"
set tig_channel(58)	"SOUND-3.74"
set tig_channel(59)	"SOUND-VIS"

set tig_res_suffix	"km"

cd $gempak_datadir
file mkdir $gempak_satdir
file mkdir $gempak_sattmpdir

set params [eval "exec $nbspsat_tool $nbspsat_opts [lindex $argv 0]"]

set sector [lindex $params 2]
set channel [lindex $params 3]
set res [lindex $params 4]
set time [lindex $params 5]
set fname [lindex $params 6]

set dir $tig_sector($sector)/$res$tig_res_suffix/$tig_channel($channel)
set gempak_fname $tig_channel($channel)_$time
file mkdir $gempak_satdir/$dir

file rename -force $gempak_sattmpdir/$fname $gempak_satdir/$dir/$gempak_fname

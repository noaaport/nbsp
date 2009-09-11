#!%TCLSH%
#
# $Id$
#
package require fileutil;

## The common grib tools initialization
set initfile "/usr/local/libexec/nbsp/nbspgribtools.init";
if {[file exists $initfile] == 0} {
    puts "$initfile not found.";
    return 1;
}
source $initfile;
unset initfile;

proc make_catalog {subdir fext catdir catfile} {
#
# This function assumes that the current directory has been set to
# gribfilter(datadir).
#

    if {[file isdirectory $subdir] == 0} {
	return;
    }

    set flist [lsort [split [exec find $subdir -name "*$fext"] "\n"]];
    if {[llength $flist] == 0} {
	return;
    }

    foreach _f $flist {
	# _f includes the subdir component; e.g.,
	# grb/nam/2007..../nam_<grid>_<reftime>_<forecast>.grb"
	# The extension is not included in the catalog.
	set _parts [lrange [file split ${_f}] 1 end];
	set rpath [join ${_parts} [file separator]];
	
	# Break the name using the function provided by the gribfilter
	# (in gribfilter.init)
	set fname [file rootname [file tail $rpath]];
	set fname_parts [gribfilter_break_default_name $fname];
	
	set model [lindex $fname_parts 0];
	set grid [lindex $fname_parts 1];
	set reftime [lindex $fname_parts 2];
	set ftime [lindex $fname_parts 3];

	lappend output "$model:$grid:$reftime:$ftime:$rpath";
    }

    file mkdir $catdir;
    ::fileutil::writeFile [file join $catdir $catfile] [join $output "\n"];
}

#
# main
#

cd $gribfilter(datadir);

# Pass a wildcard on the grib extension to catch grib2 also. (We could
# also use a different catalog.)
make_catalog $gribfilter(grbdatadir) "$gribfilter(grbfext)*" \
    $gribfilter(catalogdir) $gribfilter(grbcatalog); 
make_catalog $gribfilter(ctldatadir) $gribfilter(ctlfext) \
    $gribfilter(catalogdir) $gribfilter(ctlcatalog); 

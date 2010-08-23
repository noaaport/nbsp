#
# Functions to support the inventory
#

proc filter_insert_inventory {invsubdir savedir fpathout} {
# 
# See the function with this same name in the rstfilter.lib for the meaning
# of the "savedir" argument.
#
    global gisfilter;

    # The gis invdir must exist
    if {[file isdirectory $gisfilter(invdir)] == 0} {
	log_msg "No $gisfilter(invdir)";
	return;
    }

    # The inventory files are saved in hourly subdirs within
    # gis/<invsubdir>. E.g.,
    # gis/nexrad/nids/<yyyymmddhh>/nexrad.nids.mhx.ncr
    #
    set parentdir [file join $gisfilter(invdir) $invsubdir \
       [clock format [clock seconds] -format $gisfilter(invformat) -gmt true]];
    file mkdir $parentdir;

    set invfile_name [join [file split $savedir] "."]
    set invfile [file join $parentdir $invfile_name];

    filterlib_file_append $invfile $fpathout;
}

proc filter_rad_insert_inventory {savedir fpathout} {

    global gisfilter;

    if {$gisfilter(rad_inv_enable) == 0} {
	return;
    }

    filter_insert_inventory $gisfilter(radinvsubdir) $savedir $fpathout;
}

proc filter_sat_insert_inventory {savedir fpathout} {

    global gisfilter;

    if {$gisfilter(sat_inv_enable) == 0} {
	return;
    }

    filter_insert_inventory $gisfilter(satinvsubdir) $savedir $fpathout;
}

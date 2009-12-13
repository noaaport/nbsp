#
# Functions to support the inventory
#

proc filter_insert_inventory {invsubdir savedir fpathout} {
# 
# See the function with this same name in the rstfilter.lib for the meaning
# of the "savedir" argument.
#
    global dafilter;

    # The da invdir must exist
    if {[file isdirectory $dafilter(invdir)] == 0} {
	log_msg "No $dafilter(invdir)";
	return;
    }

    # The inventory files are saved in hourly subdirs within
    # digatmos/<invsubdir>. E.g.,
    # digatmos/nextad/nids/<yyyymmddhh>/nexrad.nids.mhx.ncr
    #
    set parentdir [file join $dafilter(invdir) $invsubdir \
	[clock format [clock seconds] -format $dafilter(invformat) -gmt true]];
    file mkdir $parentdir;

    set invfile_name [join [file split $savedir] "."]
    set invfile [file join $parentdir $invfile_name];

    filterlib_file_append $invfile $fpathout;
}

proc filter_rad_insert_inventory {savedir fpathout} {

    global dafilter;

    if {$dafilter(rad_inv_enable) == 0} {
	return;
    }

    filter_insert_inventory $dafilter(radinvsubdir) $savedir $fpathout;
}

proc filter_sat_insert_inventory {savedir fpathout} {

    global dafilter;

    if {$dafilter(sat_inv_enable) == 0} {
	return;
    }

    filter_insert_inventory $dafilter(satinvsubdir) $savedir $fpathout;
}

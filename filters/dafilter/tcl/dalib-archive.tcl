#
# Functions to support the archive
#
proc filter_file_archive {rc_array seq fpath savedir savename} {
#
# This function is called by filter_file.
#
    upvar $rc_array rc;
    global dafilter;

    if {[is_archive_file_rule_enabled $savedir] == 0} {
	return;
    }

    filter_archive rc $seq $fpath $savedir $savename;
}

proc filter_rad_archive {rc_array seq fpath savedir savename} {

    upvar $rc_array rc;
    global dafilter;

    if {[is_archive_rad_rule_enabled $savedir] == 0} {
	return;
    }

    filter_archive rc $seq $fpath $savedir $savename;
}


proc filter_sat_archive {rc_array seq fpath savedir savename} {

    upvar $rc_array rc;
    global dafilter;

    if {[is_archive_sat_rule_enabled $savedir] == 0} {
	return;
    }

    filter_archive rc $seq $fpath $savedir $savename;
}

proc filter_archive {rc_array seq fpath savedir savename} {
#
# This function is called by filter_file.
#
    upvar $rc_array rc;
    global dafilter;

    set archive_savedir [file join $dafilter(archive_subdir) $savedir];
    set archivepath [file join $archive_savedir $savename];

    set _pwd [pwd];
    cd $dafilter(archive_datadir);

    set status [catch {
        file mkdir $archive_savedir;

	if {$dafilter(archive_asyncmode) == 1} {
	    exec tar -r -f $archivepath -C [file dirname $fpath] \
		[file tail $fpath] &;
	} else {
	    exec tar -r -f $archivepath -C [file dirname $fpath] \
		[file tail $fpath];
	}
    } errmsg];

    cd $_pwd;

    if {$status != 0} {
	log_msg $errmsg;
	return;
    }

    filter_register_archive_inventory rc $savedir $savename;
}

proc filter_register_archive_inventory {rc_array \
	archive_savedir archive_name} {
#
# subdir is the common subdirectory of the main archive, for example 2009/12/01
# savedir is the data directory, for example, nwx/nhc
#
    upvar $rc_array rc;
    global dafilter;

    set invpath [file join $dafilter(archive_inv_subdir) \
		       $dafilter(archive_inv_name)];

    set invdata [list];
    foreach key [list station wmoid awips wmotime seconds fbasename] {
	lappend invdata $rc($key);
    }
    lappend invdata $dafilter(archive_datadir) $dafilter(archive_subdir) \
	$archive_savedir $archive_name;

    set _pwd [pwd];
    cd $dafilter(archive_invdir);

    set status [catch {
	file mkdir $dafilter(archive_inv_subdir);
	filterlib_file_append $invpath [join $invdata ","];
    } errmsg];

    cd $_pwd;

    if {$status != 0} {
	log_msg $errmsg;
    }
}

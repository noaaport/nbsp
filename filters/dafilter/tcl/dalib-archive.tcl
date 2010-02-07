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

    set _pwd [pwd];

    cd $dafilter(archive_datadir);

    set archive_savedir [file join $dafilter(archive_subdir) $savedir];
    file mkdir $archive_savedir;

    set status [catch {
        filterlib_cspool_nbspfile $seq $fpath $archive_savedir $savename "-a";
	# filterlib_nbspfile $seq $fpath $archive_savedir $savename "-a";
    } errmsg];

    cd $_pwd;

    if {$status != 0} {
	log_msg $errmsg;
	return;
    }

    filter_register_archive_inventory rc $savedir $savename;
}


proc filter_rad_archive {rc_array seq fpath savedir savename} {

    upvar $rc_array rc;
    global dafilter;

    if {[is_archive_rad_rule_enabled $savedir] == 0} {
	return;
    }

    set _pwd [pwd];

    cd $dafilter(archive_datadir);

    set archive_savedir [file join $dafilter(archive_subdir) $savedir];
    file mkdir $archive_savedir;

    set status [catch {
	filterlib_cspool_nbspfile $seq $fpath $archive_savedir $savename "-a";
	# filterlib_nbspfile $seq $fpath $archive_savedir $savename "-a";
    } errmsg];

    cd $_pwd;

    if {$status != 0} {
	log_msg $errmsg;
	return;
    }

    filter_register_archive_inventory rc $savedir $savename;
}


proc filter_sat_archive {rc_array seq fpath savedir savename} {

    upvar $rc_array rc;
    global dafilter;
    global filtersprogs;

    if {[is_archive_sat_rule_enabled $savedir] == 0} {
	return;
    }

    set _pwd [pwd];

    cd $dafilter(archive_datadir);

    set archive_savedir [file join $dafilter(archive_subdir) $savedir];
    file mkdir $archive_savedir;
    set archivepath [file join $archive_savedir $savename];

    set status [catch {
	if {$dafilter(satuncompress) == 0} {
	    file copy -force $fpath $archivepath;
	} else {
	    exec $filtersprogs(nbspunz) -o $archivepath $fpath;
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

    set invfpath [file join $dafilter(archive_invdir) \
		       $dafilter(archive_inv_subdir) \
		       $dafilter(archive_inv_name)];

    set invdata [list];
    foreach key [list station wmoid awips wmotime seconds] {
	lappend invdata $rc($key);
    }
    lappend invdata $dafilter(archive_datadir) $dafilter(archive_subdir) \
	$archive_savedir $archive_name;

    set status [catch {
	filterlib_file_append $invfpath [join $invdata ","];
    } errmsg];

    if {$status != 0} {
	log_msg $errmsg;
    }
}

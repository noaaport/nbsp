proc filter_rad {seq fpath savedir savename {level2flag 0}} {

    global dafilter;

    if {[is_rad_rule_enabled $savedir] == 0} {
	return;
    }

    if {$dafilter(archive_rad_enable) == 2} {
	# Archive-only mode
	return;
    }

    set _pwd [pwd];

    cd $dafilter(datadir);
    file mkdir $savedir;
    set data_path [file join $savedir $savename];
    set datafpath [file join $dafilter(datadir) $data_path];

    set status [catch {
	if {$level2flag == 0} {
	    filterlib_cspool_nbspfile $seq $fpath $savedir $savename;
	    # filterlib_nbspfile $seq $fpath $savedir $savename;
	} else {
	    filterlib_cspool_nbspfile $seq $fpath $savedir $savename "-t";
	}
    } errmsg];

    if {$status != 0} {
	# In case the file was created
	file delete $data_path;
	log_msg $errmsg;

	return;
    }

    filter_rad_insert_inventory $savedir $datafpath;

    # Create the link to the latest
    make_rad_latest $savedir $savename;

    # Create the directory listing
    make_rad_dirlist $savedir;

    cd $_pwd;
}

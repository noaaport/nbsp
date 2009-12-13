proc filter_rad_unzflag {seq fpath savedir savename unzflag} {

    global dafilter;
    global filtersprogs;

    if {[is_rad_rule_enabled $savedir] == 0} {
	return;
    }

    if {$dafilter(archive_rad_enable) == 2} {
	# Archive-only mode
	return;
    }

    if {$unzflag != 0} {
	# Set the options for nbspunz
	if {$dafilter(radstripheader) == 1} {
	    set opts "-c $dafilter(ccbsize)";
	} elseif {$dafilter(radstripheader) == 2} {
	    set opts "-c $dafilter(ccbwmoawipssize)";
	} else {
	    set opts "";
	}
    }

    set _pwd [pwd];

    cd $dafilter(datadir);
    file mkdir $savedir;
    set data_path [file join $savedir $savename];
    set datafpath [file join $dafilter(datadir) $data_path];

    set status [catch {
	if {$unzflag != 0} {
	    eval exec $filtersprogs(nbspunz) $opts $fpath > $data_path;
	} else {
	    filterlib_cspool_nbspfile $seq $fpath $savedir $savename;
	    # filterlib_nbspfile $seq $fpath $savedir $savename;
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

proc filter_rad {seq fpath savedir savename} {

    filter_rad_unzflag $seq $fpath $savedir $savename 0;
}

proc filter_rad_unz {seq fpath savedir savename} {

    filter_rad_unzflag $seq $fpath $savedir $savename 1;
}

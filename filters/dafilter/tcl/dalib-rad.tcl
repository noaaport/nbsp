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

    # We could use a partial path: data_path [file join $savedir $savename]
    # but the inventory uses the full path, so we use datafpath for everything.
    # set data_path [file join $savedir $savename];
    # set datafpath [file join $dafilter(datadir) $data_path];
    
    set datafpath [file join $dafilter(datadir) $savedir $savename];

    set status [catch {
	if {$level2flag == 0} {
	    filterlib_exec_nbspfile $seq $fpath $savedir $savename;
	    # filterlib_nbspfile $seq $fpath $savedir $savename;
	} else {
	    # level 2 files do not have a ccb, and the gempak header
	    # will not be inserted. It is then simpler to
	    # copy it directly rather than calling nbspfile with options.
	    file copy -force $fpath $datafpath;
	}
    } errmsg];

    if {$status != 0} {
	# In case the file was created
	file delete $datafpath;
	log_msg $errmsg;	
    }

    if {$status == 0} {
	filter_rad_insert_inventory $savedir $datafpath;

	# Create the link to the latest
	make_rad_latest $savedir $savename $level2flag;
	
	# Create the directory listing
	make_rad_dirlist $savedir $level2flag;
    }
    
    cd $_pwd;
}

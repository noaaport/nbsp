#
# $Id$
#
proc filter_rad {rc_varname} {

    global gisfilter;
    upvar $rc_varname rc;

    # the data file is saved if the filters accepts the file
    filter_rad_create_nids rc;

    # only those in the bundles configured are converted to gis formats
    foreach bundle $gisfilter(rad_bundlelist) {
	set regex $gisfilter(rad_bundle,$bundle,regex);
	if {[filterlib_uwildmat $regex $rc(awips)] == 0} {
	    continue;
	}
	filter_rad_convert_nids_shp rc $bundle;
    }
}

proc filter_rad_create_nids {rc_varname} {
    #
    # Write the nids file
    #
    global gisfilter;
    upvar $rc_varname rc;

    set seq $rc(seq);
    set fpath $rc(fpath);

    set data_savedir [subst $gisfilter(rad_outputfile_dirfmt,nids)];
    set data_savename [subst $gisfilter(rad_outputfile_namefmt,nids)];

    file mkdir $data_savedir;
    set data_path [file join $data_savedir $data_savename];
    set datafpath [file join $gisfilter(datadir) $data_path];

    set status [catch {
	#
	# The files are saved without the gempak header/footer (-t).
	#
	filterlib_exec_nbspfile $seq $fpath $data_savedir $data_savename -t;

	# [NOTE (02 nov 2024): Before the removal of the cspool,
	# the -w flag was passed to "filterlib_cspool_nbspfile" to wait for
	# the nids file to be created (i.e., not use background processing)
	# before continuing to create the gis files. This flag is no longer
	# an issue because the default (which was always the case) is _not_ to
	# use background processing.]
	#
	## filterlib_cspool_nbspfile \
	##    $seq $fpath $data_savedir $data_savename -t -w;
	## filterlib_nbspfile $seq $fpath $data_savedir $data_savename -t;
	#
    } errmsg];

    if {$status != 0} {
        file delete $data_path;
        log_msg $errmsg;
        return -code error $errmsg;
    }

    filter_rad_insert_inventory $data_savedir $datafpath;
    make_rad_latest $data_savedir $data_savename;

    # Reuse the rc(fpath) variable to point to the newly created nids file.
    set rc(fpath) $datafpath;
}

proc filter_rad_convert_nids_shp {rc_varname bundle} {

    global gisfilter;
    upvar $rc_varname rc;

    set fmtlist $gisfilter(rad_bundle,$bundle,fmt);
    set nidsfpath $rc(fpath);

    # nbspradgis command line options
    set option(dbf) "-f";
    set option(info) "-o";
    set option(shp) "-p";
    set option(shx) "-x";
    set option(csv) "-v";
    set cmd_options [list];
 
   foreach fmt $fmtlist {
	set data_savedir($fmt) [subst $gisfilter(rad_outputfile_dirfmt,$fmt)];
	set data_savename($fmt) \
	    [subst $gisfilter(rad_outputfile_namefmt,$fmt)];
	file mkdir $data_savedir($fmt);
	set data_path($fmt) \
	    [file join $data_savedir($fmt) $data_savename($fmt)];
	set datafpath($fmt) [file join $gisfilter(datadir) $data_path($fmt)];

	if {[info exists option($fmt)]} {
	    lappend cmd_options $option($fmt) $datafpath($fmt);
	}
    }

    # If the nids were saved with the gempak header then we would have to use
    # -c $filterslib(gmpk_header_size).
    # NOTE: We do not pass the [-b] flag because we want to catch
    # the error and retry.
    #   set cmd [concat [list nbspradgis -b -D] $cmd_options $nidsfpath];
    #
    set cmd [concat [list nbspradgis -D] $cmd_options $nidsfpath];
    set status [catch {
	eval exec $cmd;
    } errmsg];

    # In case of error try with nbspunz
    # (See note in "gisfilter/dev-notes/nids.README")
    #
    if {$status != 0} {
	set cmd [concat [list nbspunz -C $nidsfpath | nbspradgis -D] \
		     $cmd_options];
	set status [catch {
	    eval exec $cmd;
	} errmsg];
    }

    # Append the wmo header data to the info file (if "info" is in fmtlist)
    if {([lsearch $fmtlist info] != -1) && ($status == 0)} {
	set infodata "rootname: [file rootname $data_savename(info)]\n";
	foreach k [list awips] {
	    append infodata "$k: $rc($k)\n";
	}

	set status [catch {
	    ::fileutil::appendToFile $datafpath(info) $infodata;
	} errmsg];
    }

    if {$status != 0} {
	foreach fmt $fmtlist {
	    file delete $datafpath($fmt);
	}
	log_msg $errmsg;
	return;
    }

    # Insert each one in the cleanup inventory and create the latest link
    foreach fmt $fmtlist {
	filter_rad_insert_inventory $data_savedir($fmt) $datafpath($fmt);
	if {$gisfilter(rad_latest_enable) != 0} {
	    make_rad_latest $data_savedir($fmt) $data_savename($fmt);
	}
    }
}

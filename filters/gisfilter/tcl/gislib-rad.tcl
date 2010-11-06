#
# $Id$
#
proc filter_rad {rc_varname} {

    global gisfilter;
    upvar $rc_varname rc;

    filter_rad_create_nids rc;

    foreach bundle $gisfilter(rad_bundlelist) {
	set regex $gisfilter(rad_bundle,$bundle,regex);
	if {[filterlib_uwildmat $regex $rc(fname)] == 0} {
	    continue;
	}
	#
	# shp conversion will be done on-the-fly here, all the others
	# will be dispatched to an external program (WCT).
	#
	set fmt $gisfilter(rad_bundle,$bundle,outputfile_fmt);
	if {$fmt eq "shp"} {
	    filter_rad_convert_nids_shp rc $bundle;
	} else {
	    filter_rad_queue_convert_nids rc $bundle;
	}
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
	# The files are saved without the gempak header/footer (-t)
	#
	filterlib_cspool_nbspfile $seq $fpath $data_savedir $data_savename -t;
	## filterlib_nbspfile $seq $fpath $data_savedir $data_savename -t;
    } errmsg];

    if {$status != 0} {
        file delete $data_path;
        log_msg $errmsg;
        return -code error $errmsg;
    }

    filter_rad_insert_inventory $data_savedir $datafpath;
    make_rad_latest $data_savedir $data_savename;

    # Reuse the rc(fpath) variable to point to the newly created gini file.
    set rc(fpath) $datafpath;
}

proc filter_rad_convert_nids_wct_unused {rc_varname bundle} {

    global gisfilter;
    upvar $rc_varname rc;

    set nidsfpath $rc(fpath);
    set wctrcfile $gisfilter(rad_bundle,$bundle,wctrc_file);

    # shorthand
    set fmt $gisfilter(rad_bundle,$bundle,outputfile_fmt);

    set data_savedir [subst $gisfilter(rad_outputfile_dirfmt,$fmt)];
    set data_savename [subst $gisfilter(rad_outputfile_namefmt,$fmt)];

    file mkdir $data_savedir;
    set data_path [file join $data_savedir $data_savename];
    set datafpath [file join $gisfilter(datadir) $data_path];

    set cmd [list "nbspwct" -b -f $fmt -t rad -x $wctrcfile];
    if {$gisfilter(wct_debug) != 0} {
	lappend cmd "-V";
    }
    if {$gisfilter(rad_latest_enable) != 0} {
	lappend cmd -l $gisfilter(rad_latestname);
    }
    lappend cmd -d $data_savedir -o $data_savename $nidsfpath;

    # Because WCT takes some time to process the file, we will execute it
    # in the background. 

    eval exec $cmd &;

    # Because of the background execution there is no way to know here if
    # WCT actially succeeded, so we will insert it in the inventory anyway.
    # filter_rad_insert_inventory $data_savedir $datafpath;
}

proc filter_rad_queue_convert_nids {rc_varname bundle} {

    global gisfilter;
    upvar $rc_varname rc;

    set nidsfpath $rc(fpath);
    set wctrcfile $gisfilter(rad_bundle,$bundle,wctrc_file);

    # shorthand
    set fmt $gisfilter(rad_bundle,$bundle,outputfile_fmt);

    set data_savedir [subst $gisfilter(rad_outputfile_dirfmt,$fmt)];
    set data_savename [subst $gisfilter(rad_outputfile_namefmt,$fmt)];

    file mkdir $data_savedir;
    set data_path [file join $data_savedir $data_savename];
    set datafpath [file join $gisfilter(datadir) $data_path];

    # Insert it in the inventory unconditionally. This actually
    # almost usless because the wct output file is sometimes accompanied
    # by several other files, and trying to anticipate here all of them
    # is doomed to fail at some point.
    #
    # filter_rad_insert_inventory $data_savedir $datafpath;

    # Write to the wct list
    lappend gisfilter(wct_listfile_list,rad,$fmt) \
	"$nidsfpath,[file dirname $datafpath],$wctrcfile" \
	"#,rad,$nidsfpath,$datafpath";

    # Write the file if the current listfile expired
    set wct_listfile $gisfilter(wct_listfile_fpath,rad,$fmt);
    set next_wct_listfile [filter_make_next_qf "rad" $fmt];
    
    if {($next_wct_listfile ne $wct_listfile) || \
	($gisfilter(wct_listfile_flush) == 1)} {

	# Use this function instead of ::fileutil::appendToFile to ensure
	# that there is a newline inserted at the end so that if further
	# appends are made thay will go in a new line.

	filterlib_file_append $wct_listfile \
	    [join $gisfilter(wct_listfile_list,rad,$fmt) "\n"];

	set gisfilter(wct_listfile_list,rad,$fmt) [list];

	if {$next_wct_listfile ne $wct_listfile} {
	    filter_rad_process_listfile $wct_listfile $fmt;
	    set gisfilter(wct_listfile_fpath,rad,$fmt) $next_wct_listfile;
	}
    }
}

proc filter_rad_convert_nids_shp {rc_varname bundle} {

    global gisfilter;
    upvar $rc_varname rc;

    set fmt $gisfilter(rad_bundle,$bundle,outputfile_fmt);
    if {$fmt ne "shp"} {
        return -code error "filter_rad_convert_nids_shp called incorrectly.";
    }

    set nidsfpath $rc(fpath);

    # shorthand
    set fmtlist [list shp shx dbf info];

    foreach fmt $fmtlist {
	set data_savedir($fmt) [subst $gisfilter(rad_outputfile_dirfmt,$fmt)];
	set data_savename($fmt) \
	    [subst $gisfilter(rad_outputfile_namefmt,$fmt)];
	file mkdir $data_savedir($fmt);
	set data_path($fmt) \
	    [file join $data_savedir($fmt) $data_savename($fmt)];
	set datafpath($fmt) [file join $gisfilter(datadir) $data_path($fmt)];
    }

    #
    # The ccb, wmo/awips, gempak headers must be removed for nbsdcnids.
    # The -F option instructs nbspdcnids to apply the built-in filtering
    # options of the data (e.g., ignoring polygons with level values
    # less than 1.
    #
    if {[regexp $gisfilter(rad_unz) $rc(awips1)]} {
	set cmd [list nbspunz -c $gisfilter(rad_totalheadersize) $nidsfpath \
		     | nbspdcnids -b -F \
		     -p $datafpath(shp) \
		     -x $datafpath(shx) \
		     -f $datafpath(dbf)];
    } else {
	#
	# If the nids are saved with the gempak header then we have to use
	# -c $gisfilter(rad_wmoawipsgmpk_header_size)
	#
	set cmd [list nbspdcnids -b -F \
		     -c $gisfilter(rad_wmoawips_size) \
		     -p $datafpath(shp) \
		     -x $datafpath(shx) \
		     -f $datafpath(dbf) \
		     $nidsfpath];
    }

    set status [catch {
	eval exec $cmd;
    } errmsg];

    if {$status == 0} {
	# Write the info file
	set infodata "rootname: [file rootname $data_savename(info)]\n";
	foreach k [list awips radseconds radmode] {
	    append infodata "$k: $rc($k)\n";
	}

	set status [catch {
	    ::fileutil::writeFile $datafpath(info) $infodata;
	} errmsg];
    }

    if {$status != 0} {
	foreach fmt $fmtlist {
	    file delete $datafpath($fmt);
	}
	log_msg $errmsg;
	return;
    }

    # Insert each one in the cleanup inventory
    foreach fmt $fmtlist {
	filter_rad_insert_inventory $data_savedir($fmt) $datafpath($fmt);
    }

    if {$gisfilter(rad_latest_enable) != 0} {
	# Create the link to the info file
	make_rad_latest $data_savedir(info) $data_savename(info);
    }    
}

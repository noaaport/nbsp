#
# $Id$
#
proc filter_sat {rc_varname} {

    global gisfilter;
    upvar $rc_varname rc;

    filter_sat_create_gini rc;

    foreach bundle $gisfilter(sat_bundlelist) {
	set regex $gisfilter(sat_bundle,$bundle,regex);
	if {[filterlib_uwildmat $regex $rc(fname)] == 0} {
	    continue;
	}
	filter_sat_queue_convert_gini rc $bundle;
    }
}

proc filter_sat_create_gini {rc_varname} {
    #
    # Write the gini file
    #
    global gisfilter;
    upvar $rc_varname rc;

    set data_savedir [subst $gisfilter(sat_outputfile_dirfmt,gini)];
    set data_savename [subst $gisfilter(sat_outputfile_namefmt,gini)];

    file mkdir $data_savedir;
    set data_path [file join $data_savedir $data_savename];
    set datafpath [file join $gisfilter(datadir) $data_path];

    set status [catch {
	exec nbspunz -o $data_path $rc(fpath);
    } errmsg];

    if {$status != 0} {
        file delete $data_path;
        log_msg $errmsg;
        return -code error $errmsg;
    }

    filter_sat_insert_inventory $data_savedir $datafpath;
    make_sat_latest $data_savedir $data_savename;

    # Reuse the rc(fpath) variable to point to the newly created gini file.
    set rc(fpath) $datafpath;
}

proc filter_sat_convert_gini {rc_varname bundle} {

    global gisfilter;
    upvar $rc_varname rc;

    set ginifpath $rc(fpath);
    set wctrcfile $gisfilter(sat_bundle,$bundle,wctrc_file);

    # shorthand
    set fmt $gisfilter(sat_bundle,$bundle,outputfile_fmt);

    set data_savedir [subst $gisfilter(sat_outputfile_dirfmt,$fmt)];
    set data_savename [subst $gisfilter(sat_outputfile_namefmt,$fmt)];

    file mkdir $data_savedir;
    set data_path [file join $data_savedir $data_savename];
    set datafpath [file join $gisfilter(datadir) $data_path];

    set cmd [list "nbspwct" -b -f $fmt -t sat -x $wctrcfile];
    if {$gisfilter(wct_debug) != 0} {
	lappend cmd "-V";
    }
    if {$gisfilter(sat_latest_enable) != 0} {
	lappend cmd -l $gisfilter(sat_latestname);
    }
    lappend cmd -d $data_savedir -o $data_savename $ginifpath;

    # Because WCT takes some time to process the file, we will execute it
    # in the background. 

    eval exec $cmd &;

    # Because of the background execution there is no way to know here if
    # WCT actually succeeded, so we will insert it in the inventory anyway.

    filter_sat_insert_inventory $data_savedir $datafpath;
}

proc filter_sat_queue_convert_gini {rc_varname bundle} {

    global gisfilter;
    upvar $rc_varname rc;

    set ginifpath $rc(fpath);
    set wctrcfile $gisfilter(sat_bundle,$bundle,wctrc_file);

    # shorthand
    set fmt $gisfilter(sat_bundle,$bundle,outputfile_fmt);

    set data_savedir [subst $gisfilter(sat_outputfile_dirfmt,$fmt)];
    set data_savename [subst $gisfilter(sat_outputfile_namefmt,$fmt)];

    file mkdir $data_savedir;
    set data_path [file join $data_savedir $data_savename];
    set datafpath [file join $gisfilter(datadir) $data_path];

    # Write to the wct list
    lappend gisfilter(wct_listfile_list,$fmt)
	"$ginifpath,[file dirname $datafpath],$wctrcfile" \
	"#,$ginifpath,$datafpath";

    # Write the file if the current minute expired
    set current_minute [clock format [clock seconds] -format "%M"];
    if {$current_minute ne $gisfilter(wct_listfile_minute)} {
	append wct_listfile_name $fmt "." $gisfilter(wct_listfile_minute);
	set wct_listfile [file join \
	    $gisfilter(wct_listfile_qdir) $wct_listfile_name];

	::fileutil::appendToFile $wct_listfile \
	    [join $gisfilter(wct_listfile_list,$fmt) "\n"];

	# reinitialize
	set gisfilter(wct_listfile_minute) $current_minute;
	set gisfilter(wct_listfile_list,$fmt) [list];
    }
#    puts "$ginifpath,[file dirname $datafpath],$wctrcfile";
#    puts "#,$ginifpath,$datafpath";
}

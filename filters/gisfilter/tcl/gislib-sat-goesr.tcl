#
# $Id$
#
proc filter_sat {rc_varname} {

    global gisfilter;
    upvar $rc_varname rc;

    filter_sat_create_goesr rc;

    foreach bundle $gisfilter(sat_bundlelist) {
	set regex $gisfilter(sat_bundle,$bundle,regex);
	if {[filterlib_uwildmat $regex $rc(wmoid)] == 0} {
	    continue;
	}
	filter_sat_convert_goesr rc $bundle;
    }
}

proc filter_sat_create_goesr {rc_varname} {
    #
    # Write a copy of the data (goesr) file
    #
    global gisfilter;
    upvar $rc_varname rc;

    set data_savedir [subst $gisfilter(sat_outputfile_dirfmt,goesr)];
    set data_savename [subst $gisfilter(sat_outputfile_namefmt,goesr)];

    file mkdir $data_savedir;
    set data_path [file join $data_savedir $data_savename];
    set datafpath [file join $gisfilter(datadir) $data_path];

    set status [catch {
	# See comment in filter_sat{} in dalib-sat.tcl
	# exec tail -n +2 $rc(fpath) > $data_path
	filterlib_exec_nbspfile 0 $rc(fpath) $data_savedir $data_savename "-w";
    } errmsg];

    if {$status != 0} {
        file delete $data_path;
        log_msg $errmsg;
        return -code error $errmsg;
    }

    filter_sat_insert_inventory $data_savedir $datafpath;
    make_sat_latest $data_savedir $data_savename;

    # Reuse the rc(fpath) variable to point to the newly created goesr file.
    set rc(fpath) $datafpath;
}

proc filter_sat_convert_goesr {rc_varname bundle} {

    global gisfilter;
    upvar $rc_varname rc;

    set fmtlist $gisfilter(sat_bundle,$bundle,fmt);
    set goesrfpath $rc(fpath);

    # nbspgoesrgis command line options
    set option(asc) "-a";
    set option(dbf) "-f";
    set option(info) "-o";
    set option(shp) "-p";
    set option(shx) "-x";
    set option(csv) "-v";
    set cmd_options [list];

    foreach fmt $fmtlist {
	set data_savedir($fmt) [subst $gisfilter(sat_outputfile_dirfmt,$fmt)];
	set data_savename($fmt) \
	    [subst $gisfilter(sat_outputfile_namefmt,$fmt)];
	file mkdir $data_savedir($fmt);
	set data_path($fmt) \
	    [file join $data_savedir($fmt) $data_savename($fmt)];
	set datafpath($fmt) [file join $gisfilter(datadir) $data_path($fmt)];

	if {[info exists option($fmt)]} {
	    lappend cmd_options $option($fmt) $datafpath($fmt);
	}
    }

    set cmd [concat [list nbspgoesrgis -b] $cmd_options $goesrfpath];

    set status [catch {
	eval exec $cmd;
    } errmsg];

    # Append the wmo header data to the info file (if "info" is in fmtlist)
    if {([lsearch $fmtlist info] != -1) && ($status == 0)} {
	set infodata "rootname: [file rootname $data_savename(info)]\n";
	foreach k [list wmoid] {
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
	filter_sat_insert_inventory $data_savedir($fmt) $datafpath($fmt);
	if {$gisfilter(sat_latest_enable) != 0} {
	    make_sat_latest $data_savedir($fmt) $data_savename($fmt);
	}
    }
}

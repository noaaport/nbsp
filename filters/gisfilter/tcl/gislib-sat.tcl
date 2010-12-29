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
	filter_sat_convert_gini_shp rc $bundle;
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
        # Sat files do not have a ccb
	file copy -force $rc(fpath) $data_path;
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

proc filter_sat_convert_gini_shp {rc_varname bundle} {

    global gisfilter;
    upvar $rc_varname rc;

    set fmtlist $gisfilter(sat_bundle,$bundle,fmt);
    set ginifpath $rc(fpath);

    # nbspginishp command line options
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

    set cmd [concat [list nbspunz $ginifpath | nbspginishp -b] $cmd_options];

    set status [catch {
	eval exec $cmd;
    } errmsg];

    if {$status == 0} {
	# Append the wmo header data to the info file
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

    # Insert each one in the cleanup inventory
    foreach fmt $fmtlist {
	filter_sat_insert_inventory $data_savedir($fmt) $datafpath($fmt);
    }

    if {$gisfilter(sat_latest_enable) != 0} {
	# Create the link to the info file
	make_sat_latest $data_savedir(info) $data_savename(info);
    }
}

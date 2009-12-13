#
# Function to support the directory listing
#
proc make_rad_dirlist {dir} {

    global dafilter;

    if {$dafilter(rad_dirlist_enable) == 0} {
	return;
    }

    set dirpath [file join $dafilter(datadir) $dir];
    set dirlistpath [file join $dirpath $dafilter(rad_dirlistname)];
    set filelist [lsort \
	[glob -nocomplain -tails -directory $dirpath "*"]];

    # Delete the <latest> and <dir list> files. 
    foreach f [list $dafilter(rad_dirlistname) $dafilter(rad_latestname)] {
	set i [lsearch -sorted -exact $filelist $f];
	if {$i >= 0} {
	    set filelist [lreplace $filelist $i $i];
	}
    }

    set status [catch {
        set F [open $dirlistpath w 0644];
	puts $F [join $filelist "\n"];
    } errmsg];
    if {[info exists F]} {
    	close $F;
    }

    if {$status != 0} {
        log_msg $errmsg;
    }
}

proc make_rad_latest {savedir savename} {

    global dafilter;

    if {$dafilter(rad_latest_enable) == 0} {
	return;
    }

    make_latest $savedir $savename $dafilter(rad_latestname);
}

proc make_sat_latest {savedir savename} {

    global dafilter;

    if {$dafilter(sat_latest_enable) == 0} {
	return;
    }

    make_latest $savedir $savename $dafilter(sat_latestname);
}

proc make_latest {savedir savename latestname} {
#
# Create a link to the newest file (this is for applications like DA, IDV).
#
    global dafilter;

    set currentdir [pwd];

    set dirpath [file join $dafilter(datadir) $savedir];
    cd $dirpath;

    set latest $savename;
    set linkpath $latestname;
    if {[file exists $latest] == 0} {
 	cd $currentdir;
	return;
    }

    set status [catch {
        file delete $linkpath;
        # file link -symbolic $linkpath $latest;
	exec ln -s $latest $linkpath;
    } errmsg];

    cd $currentdir;
}

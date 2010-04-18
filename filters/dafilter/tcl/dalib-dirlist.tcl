#
# Function to support the directory listing
#
proc make_rad_dirlist {dir {level2flag 0}} {

    global dafilter;

    set rad_dirlistname "";

    if {$level2flag == 0} {
	if {$dafilter(rad_dirlist_enable) != 0} {
	    set rad_dirlistname $dafilter(rad_dirlistname);
	    set rad_latestname $dafilter(rad_latestname);
	}
    } elseif {$dafilter(rad2_dirlist_enable) != 0} {
	set rad_dirlistname $dafilter(rad2_dirlistname);
	set rad_latestname $dafilter(rad2_latestname);
    }

    if {$rad_dirlistname eq ""} {
	return;
    }

    set dirpath [file join $dafilter(datadir) $dir];
    set dirlistpath [file join $dirpath $rad_dirlistname];
    set filelist [lsort \
	[glob -nocomplain -tails -directory $dirpath "*"]];

    # Delete the <latest> and <dir list> files.
    foreach f [list $rad_dirlistname $rad_latestname] {
	set i [lsearch -sorted -exact $filelist $f];
	if {$i >= 0} {
	    set filelist [lreplace $filelist $i $i];
	}
    }

    # For level 2, prepend the file size to each file name in the list
    # (without the file size, grlevel2 would not pull any data)
    # Brad Holcomb <brad@w0wdx.com> 4/13/2010

    if {$level2flag != 0} {
	set oldlist $filelist;
	set filelist [list];
	foreach f $oldlist {
	    set s [file size [file join $dirpath $f]];
	    append s " " $f;
	    lappend filelist $s;
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

proc make_rad_latest {savedir savename {level2flag 0}} {

    global dafilter;

    set rad_latestname "";

    if {$level2flag == 0} {
	if {$dafilter(rad_latest_enable) != 0} {
	    set rad_latestname $dafilter(rad_latestname);
	}
    } elseif {$dafilter(rad2_latest_enable) != 0} {
	set rad_latestname $dafilter(rad2_latestname);
    }

    if {$rad_latestname eq ""} {
	return;
    }
    
    make_latest $savedir $savename $rad_latestname;
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

proc make_rad_latest {savedir savename {level2flag 0}} {

    global gisfilter;

    set rad_latestname "";

    if {$level2flag == 0} {
	if {$gisfilter(rad_latest_enable) != 0} {
	    set rad_latestname $gisfilter(rad_latestname);
	}
    } elseif {$gisfilter(rad2_latest_enable) != 0} {
	set rad_latestname $gisfilter(rad2_latestname);
    }

    if {$rad_latestname eq ""} {
	return;
    }
    
    make_latest $savedir $savename $rad_latestname;
}

proc make_sat_latest {savedir savename} {

    global gisfilter;

    if {$gisfilter(sat_latest_enable) == 0} {
	return;
    }

    make_latest $savedir $savename $gisfilter(sat_latestname);
}

proc make_latest {savedir savename latestname} {
#
# Create a link to the newest file
#
    global gisfilter;

    set currentdir [pwd];

    set dirpath [file join $gisfilter(datadir) $savedir];
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

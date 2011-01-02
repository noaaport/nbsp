#
# $Id$
#

# These functions are need in the filter in order to create the latest
# file for the data (gini) file. The nbspwct program has its own copy
# of the function in order to create the link for the data files produced
# by WCT.

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

    if {($gisfilter(sat_latest_enable) == 0) || \
	($gisfilter(sat_latestname) eq "")} {

	return;
    }

    make_latest $savedir $savename $gisfilter(sat_latestname);
}

proc make_latest {savedir savename latestname} {
#
# Create a link to the newest file.
#
    append linkpath $latestname [file extension $savename];
    set latest $savename;

    set currentdir [pwd];
    cd $savedir;

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

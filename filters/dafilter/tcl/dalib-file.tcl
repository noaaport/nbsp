proc filter_file {rc_array seq fpath savedir savename {pass_through 0}} {
#
# This function is also called from the filter_grib function, and there
# it is called with pass_through = 1 because the grib regex has already
# being checked.
#
    upvar $rc_array rc;
    global dafilter;

    if {$pass_through == 0} {
        if {[is_file_rule_enabled $savedir] == 0} {
	    return;
        }
    }

    if {$dafilter(archive_file_enable) != 2} {
	filter_file_normal $seq $fpath $savedir $savename "-a";
    }

    if {$dafilter(archive_file_enable) != 0} {
	filter_file_archive rc $seq $fpath $savedir $savename "-a";
    }
}

proc filter_file_noappend {seq fpath savedir savename} {

    global dafilter;
    global filtersprogs;

    if {[is_file_rule_enabled $savedir] == 0} {
	return;
    }

    if {$dafilter(archive_file_enable) != 2} {
	filter_file_normal $seq $fpath $savedir $savename;
    }
}

proc filter_file_normal {seq fpath savedir savename {f_append ""}} {
#
# This is the ordinary method for saving the files under the "data" directory.
# This function is caled by filter_file and filter_file_noappend.
#
    global dafilter;

    cd $dafilter(datadir);
    file mkdir $savedir;

#    set opts $f_append;
#    if {$dafilter(ccbsaved) == 0} {
#        append opts " -n";
#    }

    set status [catch {
	if {$f_append eq "-a"} {
	    filterlib_cspool_nbspfile $seq $fpath $savedir $savename "-a";
#	    filterlib_nbspfile $seq $fpath $savedir $savename "-a";
#           eval exec nbspfile $opts -d $savedir -o $savename $fpath $seq;
	} else {
	    filterlib_cspool_nbspfile $seq $fpath $savedir $savename;
#	    filterlib_nbspfile $seq $fpath $savedir $savename;
#           eval exec nbspfile $opts -d $savedir -o $savename $fpath $seq;
 	}
    } errmsg];

    if {$status != 0} {
	log_msg $errmsg;
    }
}

proc filter_file_compress {seq fpath savedir savename} {
#
# This is no longer used (Tue Sep 23 21:03:18 AST 2008)
#
    global dafilter;
    global filtersprogs;

    if {[is_file_rule_enabled $savedir] == 0} {
	return;
    }

    cd $dafilter(datadir);
    file mkdir $savedir;

    set opts "-z 9";
    if {$dafilter(ccbsaved) == 0} {
	set opts "-z 9 -n";
    }

    set status [catch {
	eval exec $filtersprogs(nbspfile) $opts -d $savedir -o $savename \
		$fpath $seq;
    } errmsg];

    if {$status != 0} {
	log_msg $errmsg;
    }
}

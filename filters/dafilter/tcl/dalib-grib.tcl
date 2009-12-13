proc filter_grib {seq fpath savedir savename} {

    global dafilter;
    global filtersprogs;

    if {[is_grib_rule_enabled $savedir] == 0} {
	return;
    }

    if {$dafilter(archive_file_enable) != 2} {
        filter_file_normal $seq $fpath $savedir $savename "-a";	
    }
}

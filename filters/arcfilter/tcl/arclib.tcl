proc arcfilterlib_tar {rc_array archive_dir archive_rootname inv_rootname} {

    upvar $rc_array rc;
    global arcfilter;

    append archive_name $archive_rootname $arcfilter(tarfext);
    set archivepath [file join $archive_dir $archive_name];

    set _pwd [pwd];
    cd $arcfilter(datadir);

    set status [catch {
        file mkdir $archive_dir;
	exec tar -r -f $archivepath -C [file dirname $rc(fpath)] \
	    [file tail $rc(fpath)];
    } errmsg];

    cd $_pwd;

    if {$status != 0} {
	log_msg $errmsg;
	return;
    }

    append invname $inv_rootname $arcfilter(invfext);
    set invpath [file join $archive_dir $invname];

    set invdata [list];
    foreach key [list station wmoid awips wmotime seconds fbasename] {
	lappend invdata $rc($key);
    }
    lappend invdata [file join $archive_name];

    cd $arcfilter(invdir);

    set status [catch {
	file mkdir $archive_dir;
	filterlib_file_append $invpath [join $invdata ","];
    } errmsg];

    cd $_pwd;

   if {$status != 0} {
	log_msg $errmsg;
    }
}

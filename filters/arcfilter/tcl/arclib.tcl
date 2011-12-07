proc arcfilterlib_tar {rc_array archivepath invpath} {

    upvar $rc_array rc;
    global arcfilter;

    set _pwd [pwd];
    cd $arcfilter(datadir);

    set status [catch {
        file mkdir [file dirname $archivepath];
	exec tar -r -f $archivepath -C [file dirname $rc(fpath)] \
	    [file tail $rc(fpath)];
    } errmsg];

    cd $_pwd;

    if {$status != 0} {
	log_msg $errmsg;
	return;
    }

    set invdata [list];
    foreach key [list station wmoid awips wmotime seconds fbasename] {
	lappend invdata $rc($key);
    }
    lappend invdata $archivepath;

    cd $arcfilter(invdir);

    set status [catch {
	file mkdir [file dirname $invpath];
	filterlib_file_append $invpath [join $invdata ","];
    } errmsg];

    cd $_pwd;

   if {$status != 0} {
	log_msg $errmsg;
    }
}

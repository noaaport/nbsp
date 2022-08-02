proc open_lock {} {

    global LOCK nbspinlock;

    set status [catch {
	set LOCK [open $nbspinlock {WRONLY CREAT NONBLOCK}]
    } errmsg];

    if {$status != 0} {
	return -code error "Could not open $nbspinlock";
    }

    return $status;
}

proc close_lock {} {

    global LOCK nbspinlock;

    set status [catch {
	close $LOCK;
    } errmsg];

    if {$status != 0} {
	return -code error "Could not close $nbspinlock";
    }

    return $status;
}

proc infeed_lock {} {

    global LOCK nbspinlock;
    
    set status [catch {
	flock -write $LOCK;
    } errmsg];

    if {$status != 0} {
	return -code error "Could not lock $nbspinlock";
    }

    return $status;
}

proc infeed_unlock {} {

    global LOCK nbspinlock;
    
    set status [catch {
	funlock $LOCK;
    } errmsg];

    if {$status != 0} {
	return -code error "Could not unlock $nbspinlock";
    }

    return $status;
}

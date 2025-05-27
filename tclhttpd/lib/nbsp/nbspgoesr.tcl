#
# $Id$
#
Direct_Url /nbspgoesr nbspgoesr

package require html

proc nbspgoesr/latest {goesrdir imgdir outputname maxage {loopflag 0}} {

    global Config;

    set savedir [pwd];
    cd $Config(docRoot);

    ::html::init;
    append result [::html::head "Latest satellite"] "\n";
    append result [::html::bodyTag] "\n";

    append result "<h1>Latest sat maps</h1>\n";

    # get the tix subdirs (exclude ixt), and for each get the tixxnn subdirs.
    set tixlist [lsort [glob -directory $goesrdir -nocomplain -tails "ti*"]];
    
    foreach tix $tixlist {
	set tixxnnlist [lsort [glob -directory [file join $goesrdir $tix] \
				  -nocomplain -tails "*"]];
	foreach wmoid $tixxnnlist {
	    set wmoid [string trim $wmoid];
	    set type [string range $wmoid 0 3]; # tire, tiur ...
	    set code [string range $wmoid 4 5]; # the numeric code: 01, 02, ...
	    lappend wmoidlist($type) $code;
	}
    }
    
    foreach type [array names wmoidlist] {
	append result "<h3>[string toupper $type]</h3>\n";
	foreach code $wmoidlist($type) {
	    set code [string trim $code];
	    set wmoid ${type}${code};
	    set wmoid02 [string range $wmoid 0 2];
	    if {[file isdirectory [file join $goesrdir $wmoid02 $wmoid]]} {
		append result "<a href=display_goesrmap?wmoid=$wmoid&imgdir=$imgdir&outputname=$outputname&maxage=$maxage&loopflag=$loopflag>$code</a>\n";
	    }
	}
	append result "<br>\n";
    }

    append result [::html::end];
    cd $savedir;
    
    return $result;
}

proc nbspgoesr/display_goesrmap {wmoid imgdir outputname maxage {loopflag 0}} {

    global Config;

    ::html::init;
    append result [::html::head "Latest sat map"] "\n";
    append result [::html::bodyTag] "\n";

    set savedir [pwd];
    cd $Config(docRoot);

    if {[file isdirectory $imgdir] == 0} {
	append "$imgdir not a directory. Configuration error.";
	append result [::html::end];

	return $result;
    }

    set outputdir [file join $imgdir $wmoid];
    file mkdir $outputdir;

    # Output is in $outputname in $outputdir
    set fpathout [file join $outputdir $outputname];

    # Lock file
    set fpathlock ${fpathout}.lock;
    set lockF [_nbspgoesr_open_lock $fpathlock 1];

    # If the file already exists and it is younger than maxage,
    # then it is not re-created.
    set recreate 1;

    if {[file exists $fpathout]} {
	set status [catch {
	    set now [clock seconds];
	    set mtime [file mtime $fpathout];
	    set age [expr $now - $mtime];
	}];

	if {($status == 0) && ($age < $maxage)} {
	    set recreate 0;
	}
    }

    if {$recreate == 1} {
	set status [catch {
	    if {$loopflag == 0} {
		exec nbspgoesrmapc -m -d $outputdir -o $outputname $wmoid;
	    } else {
		exec nbspgoesrmapc -L -m -d $outputdir -o $outputname $wmoid;
	    }
	} errmsg];
    }

    _nbspgoesr_close_lockfile $fpathlock $lockF;    

    if {[file exists $fpathout] == 0} {
	append result "Could not generate $wmoid map.";
    } else {
	set fpathout_url [file join "/" $fpathout]; 
	append result [::html::h1 "Latest $wmoid map"];
	append result "<img src=$fpathout_url>\n";
    }
    append result [::html::end];
    cd $savedir;

    return $result;
}

proc _nbspgoesr_open_lock {lockfile lock_sleep_s} {

    set locked 0; ;

    while {$locked == 0} {
	# Assume that the OS guarantees that this is an atomic operation
	set status [catch {
	    set F [open $lockfile {WRONLY CREAT EXCL}];
	} errmsg];
	
	if {$status == 0} {
	    set locked 1;
	} else {
	    # puts "Waiting for $lockfile";
	    after [expr $lock_sleep_s * 1000];
	}
    }

    return $F;
}

proc _nbspgoesr_close_lockfile {lockfile F} {

    catch {close $F};
    file delete $lockfile;
}

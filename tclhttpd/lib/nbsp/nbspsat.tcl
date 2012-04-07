#
# $Id$
#
Direct_Url /nbspsat nbspsat

package require html

proc nbspsat/latest {ginidir imgdir outputname maxage {loopflag 0}} {

    global Config;

    # Actually these will be overriden dynamically below
    set wmoid [list];

    set savedir [pwd];
    cd $Config(docRoot);

    # Construct the list of wmoid's dynamically (because the user
    # can configure the format of the directory names.
    set wmoid [lsort [glob -directory $ginidir -nocomplain -tails "*"]];

    ::html::init;
    append result [::html::head "Latest satellite"] "\n";
    append result [::html::bodyTag] "\n";

    append result "<h1>Latest sat maps</h1>\n";

    foreach w $wmoid {
	set w [string trim $w];
	set type [string range $w 0 3];    # tige, tigw ...
	set code [string range $w 4 5];    # the numeric code: 01, 02, ...
	lappend wmoidlist($type) $code;
    }

    foreach type [array names wmoidlist] {
	append result "<h3>[string toupper $type]</h3>\n";
	foreach code $wmoidlist($type) {
	    set code [string trim $code];
	    # Include only the tig(e|w|p|q)
	    # if {[regexp {^tig(e|w|p|q)} $code] == 0} {
	    #	continue;
	    # }
	    set wmoid ${type}${code};
	    if {[file isdirectory [file join $ginidir ${wmoid}]]} {
		append result "<a href=display_satmap?wmoid=$wmoid&imgdir=$imgdir&outputname=$outputname&maxage=$maxage&loopflag=$loopflag>$code</a>\n";
	    }
	}
	append result "<br>\n";
    }

    append result [::html::end];
    cd $savedir;
    
    return $result;
}

proc nbspsat/display_satmap {wmoid imgdir outputname maxage {loopflag 0}} {

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
    set lockF [_nbspsat_open_lock $fpathlock 1];

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
		exec nbspsatmapc -d $outputdir -o $outputname $wmoid;
	    } else {
		exec nbspsatmapc -L -d $outputdir -o $outputname $wmoid;
	    }
	} errmsg];
    }

    _nbspsat_close_lockfile $fpathlock $lockF;    

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

proc _nbspsat_open_lock {lockfile lock_sleep_s} {

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

proc _nbspsat_close_lockfile {lockfile F} {

    catch {close $F};
    file delete $lockfile;
}

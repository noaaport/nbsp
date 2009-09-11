#!/usr/local/bin/tclsh8.4

set emwin_unknown_state us;

proc get_locid id {

    source locid.rc;

    if {[info exists locid($id)]} {
	set state $locid($id);
    } else {
	set state "";
    }

    return $state;
}

proc emwin_get_state fname {
    #
    # Here fname is the base name of the file. For text files it is
    # kkkk_ttaaii-xxxxxx (with varying number of x's), while
    # for images it is "ttaaii_yyyymmdd_hhmm".
    #

    global emwin_unknown_state;

    # wfo_id is the last three characters of kkkk
    set wfo_id [string range $fname 1 3];

    set fname_length [string length $rootname];

    if {$fname_length == 18} {
	# then there are 6 x's. Get the last three.
	set awips_id [string range $fname 15 17];
    } else {
	set awips_id "";
    }

    set ss [get_locid $wfo_id];
    if {$ss == ""} {
	if {$awips_id != ""} {
	    set ss [get_locid $awips_id];
	}
    }

    if {$ss == ""} {
	set ss $emwin_unknown_state;
    }

    return $ss;
}

proc emwin_fname fbasename {
    #
    # For emwin, the file name must be 8 characters followed by .TXT
    # (or some similar extension).
    # The basename of the filtered file is of the form
    # kkkk_ttaaii-xxxxxx.<key>.yyy, with either 4,5 or 6 x's for text files.
    # For images, the name of the filtered files is 
    # ttaaii_yyyymmdd_hhmm.<key>.png.
    # For text files, the emwin file name we construct
    # is xxxxxxss (with 6 x's), where SS are the state initials of the
    # station (plus the extension), or xxxxxkkk (with 5 x's), or
    # xxxxkkk (with 4 x's). For images, we write it as ttaaiiss.

    # Get the root of the name and the extension (the extension includes the .)
    scan $fbasename {%[^.]} fname;
    set extension [file extension $fbasename];
    set fname_length [string length $fname];

    if {$extension == ""} {
	return "";
    }
    
    set ss [emwin_get_state $fname];

    if {$fname_length == 18} {
	set awips [string range $fname 12 17];
	append emwinname $awips $ss $extension;
    } elseif {$fname_length == 17} {
	set awips [string range $fname 12 16];
	append emwinname $awips [string range $fname 1 3] $extension;
    } elseif {$rootname_length == 16} {
	set awips [string range $fname 12 15];
	append emwinname $awips [string range $fname 1 3] $extension;
    } else {
	if {$extension == ".txt"} {
	    append emwinname [string range $fname 5 10] $ss $extension;
	} else {
	    append emwinname [string range $fname 0 5] $ss $extension;
	}
    }

    return [string toupper $emwinname];
}


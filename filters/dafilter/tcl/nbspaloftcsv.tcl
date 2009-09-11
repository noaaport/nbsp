#!%TCLSH%
#
# $Id$
#
package require cmdline;

set usage {nbspaloftcsv [-e ext] [-d savedir] [-o outputfile]
    [-n na_char] [-s separator] [inputfile]};
set optlist {{e.arg ".csv"} {d.arg ""} {o.arg ""} {n.arg "-"} {s.arg ","}};

set start_us31(0) 0;
set start_us31(1) 4;
set start_us31(2) 8;
set start_us31(3) 16;
set start_us31(4) 24;
set start_us31(5) 32;
set start_us31(6) 40;
set start_us31(7) 48;
set start_us31(8) 55;
set start_us31(9) 62;

set end_us31(0) 2;
set end_us31(1) 7;
set end_us31(2) 15;
set end_us31(3) 23;
set end_us31(4) 31;
set end_us31(5) 39;
set end_us31(6) 47;
set end_us31(7) 54;
set end_us31(8) 61;
set end_us31(9) 68;

set start_us37(0) 0;
set start_us37(1) 4;
set start_us37(2) 10;

set end_us37(0) 2;
set end_us37(1) 9;
set end_us37(2) 16;

set start_hw31(0) 0
set start_hw31(1) 4
set start_hw31(2) 8 
set start_hw31(3) 13
set start_hw31(4) 18
set start_hw31(5) 23
set start_hw31(6) 31
set start_hw31(7) 39
set start_hw31(8) 47
set start_hw31(9) 55
set start_hw31(10) 63

set end_hw31(0) 2
set end_hw31(1) 7
set end_hw31(2) 12
set end_hw31(3) 17
set end_hw31(4) 22
set end_hw31(5) 30
set end_hw31(6) 38
set end_hw31(7) 46
set end_hw31(8) 54
set end_hw31(9) 62
set end_hw31(10) 70

set start_hw37(0) 0
set start_hw37(1) 3
set start_hw37(2) 10
set start_hw37(3) 17
set start_hw37(4) 24
set start_hw37(5) 31

set end_hw37(0) 2
set end_hw37(1) 9
set end_hw37(2) 16
set end_hw37(3) 23
set end_hw37(4) 30
set end_hw37(5) 37

set start_cn31(0) 0
set start_cn31(1) 3
set start_cn31(2) 11
set start_cn31(3) 18
set start_cn31(4) 25
set start_cn31(5) 32
set start_cn31(6) 39

set end_cn31(0) 2
set end_cn31(1) 10
set end_cn31(2) 17
set end_cn31(3) 24
set end_cn31(4) 31
set end_cn31(5) 38
set end_cn31(6) 45


proc err {s} {

    puts stderr "$argv0: $s";
    cleanup;
    exit 1;
}

proc cleanup {} {

    global fin fout;

    if {[info exists fin] && ($fin ne "stdin")} {
	close $fin;
    }

    if {[info exists fout] && ($fout ne "stdout")} {
	close $fout;
    }
}

proc cvt_record {startarray endarray times altitudes line} {
#
# start and end are arrays that indicate the starting and ending position
# of each field. The various fields are right justified at fixed positions
# (i.e., fortran-like) within the various files (but not every file has the
# same positions).
#
    global option;
    upvar $startarray start;
    upvar $endarray end;

    #
    # $result will be a list of csv strings.
    #
    set result [list];
    foreach i [lsort [array names start]] {
	set record [list];
	set value [string trim [string range $line \
				    $start($i) $end($i)]];
	if {$value eq ""} {
	    set value $option(n);
	}
	if {$i == 0} {
	    set site [string tolower $value];
	} else {
	    set alt [lindex $altitudes $i];
	    set record [concat [list $site $alt] $times \
			    [decode_parameters $value $alt]];
	    lappend result [join $record $option(s)];
	}
    }

    return $result
}

proc decode_parameters {valuestr altitude} {

    global option;

    if {[string length $valuestr] == 4} {
	set temp $option(n);
    } elseif {[string length $valuestr] < 6} {	
	return [list $option(n) $option(n) $option(n)];
    } else {
	# Get rid of initial '+' signs, and zeros. 
	set temp_val [string trimleft [string range $valuestr 4 end] "+"];
	set temp_val [string trimleft $temp_val "0"];
	if {$temp_val eq ""} {
	    set temp_val 0;
	}

	if {[regexp -- {-(\d{2})} $temp_val match s1]} {
	    set temp_val "-";
	    append temp_val [string trimleft $s1 "0"];
	    if {$temp_val eq "-"} {
		set temp_val 0;
	    }
	}

	if {$altitude > 24000} {
	    append temp "-" $temp_val;
	} else {
	    set temp $temp_val;
	}
    }

    set wdir [string trimleft [string range $valuestr 0 1] "0"];
    set wspeed [string trimleft [string range $valuestr 2 3] "0"];

    # Special cases
    if {($wdir > 50) && ($wdir != 99)} {
	set wdir [expr $wdir - 50];
	set wspeed [expr $wspeed + 100];
    } elseif {$wdir == 99} {
	set wdir $option(n);
	set wspeed 0;
    }

    # The correct direction
    if {$wdir != $option(n)} {
	set wdir [expr $wdir * 10];
    }

    return [list $wdir $wspeed $temp];
}

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

# Open the input file.
if {$argc == 1} {
    set inputfile [lindex $argv 0];
    set status [catch {
	set fin [open $inputfile "r"];
    } errmsg];
    if {$status != 0} {
	err $errmsg;
    }
} elseif {$argc == 0} {
    set fin "stdin";
} else {
    err $usage;
}
fconfigure $fin -translation binary -encoding binary;

if {$option(d) ne ""} {
    set status [catch {
	cd $option(d);
    } errmsg];
    if {$status != 0} {
	err $errmsg;
    }
}

# Get the "code" from the FB (wmo) line.
while {[gets $fin line] > 0} {
    if {[regexp {^FB([[:alnum:]]{4})} $line match s1]} {
	break;
    }
}
set code [string tolower $s1];
if {[regexp {^(us|ak)3(1|3|5)} $code]} {
    set code "us31";
} elseif {[regexp {^(us|ak)3(7|8|9)} $code]} {
    set code "us37";
} elseif {[regexp {^(hw|oc)3(1|3|5)} $code]} {
    set code "hw31";
} elseif {[regexp {^(hw|oc)3(7|8|9)} $code]} {
    set code "hw37";
} elseif {[regexp {^cn3(1|3|5)} $code]} {
    set code "cn31";
} else {
    err "code $code";
}

# Get the times
while {[gets $fin line] > 0} {
    set line [string trim $line];
    set parts [split $line];
    if {[regexp {^\u001EDATA} $line]} {
	set time_run [string trimright [lindex $line 3] "Z"];
    } elseif {[regexp {^VALID} $line]} {
	set time_valid [string trimright [lindex $line 1] "Z"];
	set time_range [string trimright [lindex $line 4] ".Z"];

	break;
    }
}
set times [list $time_run $time_valid $time_range];

# Open the outputfile
if {$option(o) eq "-"} {
    set fout stdout;
} else {
    if {$option(o) ne ""} {
	set outputfile $option(o);
    } else {
	append outputfile $time_run $option(e);
    }
    set status [catch {
	set fout [open $outputfile "a"];
    } errmsg];
    if {$status != 0} {
	err $errmsg;
    }
}
fconfigure $fout -translation binary -encoding binary;

# Start of data (including the FT line)
while {[gets $fin line] > 0} {
    set line [string trim $line];

    # In some files (e.g., us|ak, there is a blank like after the VALID, before
    # the FT, but not in others (e.g., oc). 
    if {[string trim $line] eq ""} {
	continue;
    }

    if {[regexp {^FT} $line]} {
	set parts [split $line];
	set altitudes [list];
        foreach p $parts {
            if {$p ne ""} {
                lappend altitudes $p;
            }
        }
    } elseif {[regexp {^\u001E(.*)} $line match s1]} {
	if {$code eq "us31"} {
	    set records [cvt_record start_us31 end_us31 $times $altitudes $s1];
	} elseif {$code eq "us37"} {
	    set records [cvt_record start_us37 end_us37 $times $altitudes $s1];
	} elseif {$code eq "hw31"} {
	    set records [cvt_record start_hw31 end_hw31 $times $altitudes $s1];
	} elseif {$code eq "hw37"} {
	    set records [cvt_record start_hw37 end_hw37 $times $altitudes $s1];
	} elseif {$code eq "cn31"} {
	    set records [cvt_record start_cn31 end_cn31 $times $altitudes $s1];
	}
	# $records is returned as a list of csv strings.
	puts $fout [join $records "\n"];
    } else {
	# We assume that a line not starting with the \u001E is the end of the
	# file (e.g., the usual gempak-like terminating sequence).
	break;
    }
}

cleanup;

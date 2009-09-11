#!%TCLSH%
#
# $Id$

# The file can be a .sao file or a raw file, with or without the ccb.
# If the <separator> is not given, only the data portion is written, not
# the entire record. If <file> is "-", then it reads from stdin.
#
set usage {Usage: nbspmtrcsv [<separator>] <file> | -};

# The wmo header is searched anywhere in the line, not necessarily
# at the start, in order to be usable also with the files saved with the ccb.
set wmoheader_regexp {S(P|A)[A-Z]{2}\d{2} [[:alnum:]]{4} \d{6}};
set mtrspecstart_regexp {^(METAR|SPECI) [[:alnum:]]{4} \d{6}};
set stationstart_regexp {^[[:alnum:]]{4} \d{6}};
set continuation_regexp {^[[:blank:][:graph:]]+$};
set g(OFS) "";

proc match_mtrspecstart str {

    global mtrspecstart_regexp;

    if {[regexp $mtrspecstart_regexp $str]} {
	return 1;
    }

    return 0;
}

proc match_stationstart str {

    global stationstart_regexp;

    if {[regexp $stationstart_regexp $str]} {
	return 1;
    }

    return 0;
}

proc match_continuation str {

    global continuation_regexp;

    if {[regexp {^[[:blank:][:graph:]]+$} $str]} {
	return 1;
    }

    return 0;
}

proc obdata_init data {

    global record;

    set record(obdata) $data;
    set record(rcvtime) [clock seconds];
    set record(_open) 1;

    # Split and get the station id and time from the first two elements
    set elements [split $data];
    set record(obstation) [lindex $elements 0];
    set record(obtime) [lindex $elements 1];
    set record(obday) [string range $record(obtime) 0 1];
    set record(obhour) [string range $record(obtime) 2 3];
}

proc obdata_append data {

    global record;

    append record(obdata) " " $data;

    if {[regexp {=$} $data]} {
	obdata_end;
    }
}

proc obdata_end {} {

    global record;
    global g;

    set record(_open) 0;

    set record(obdata) [string trimright $record(obdata) "="];

    if {$g(OFS) == ""} {
	puts $record(obdata);
	return;
    }

    append r $record(obstation) $g(OFS) \
	$record(wmostation) $g(OFS) \
	$record(wmocountry) $g(OFS) \
	$record(rcvtime) $g(OFS) \
	$record(wmotime) $g(OFS) \
	$record(obtime) $g(OFS) \
	$record(obday) $g(OFS) \
	$record(obhour) $g(OFS) \
	$record(obtype) $g(OFS) \
	$record(obdata);

    puts $r;
}

proc obdata_start data {

    global record;

    # Close the current record if it is open
    if {$record(_open) == 1} {
	obdata_end;
    }

    obdata_init $data;

    if {[regexp {=$} $data]} {
	obdata_end;
    }
}

#
# main
#

set record(obstation) "";
set record(obtime) "";
set record(obday) "";
set record(obhour) "";
set record(obdata) "";
set record(obtype) "";
set record(rcvtime) "";
set record(wmoid) "";
set record(wmocountry) "";
set record(wmostation) "";
set record(wmotime) "";
set record(_open) 0;

set argc [llength $argv];
if {$argc < 1} {
    puts $usage;
    exit 1;
} elseif {$argc == 1} {
    set fpath [lindex $argv 0];
} elseif {$argc == 2} {
    set g(OFS) [lindex $argv 0];
    set fpath [lindex $argv 1];
}

if {$fpath eq "-"} {
    set f stdin;
} else {
    set status [catch {set f [open $fpath "r"]} errmsg];
    if {$status != 0} {
	puts $errmsg;
	exit 1;
    }
}

fconfigure $f -encoding binary -translation binary;

while {[gets $f line] >= 0 } {

    set s [string trim $line];

    # Ignore empty lines that we get due to the ending \r\r\n sequence.
    if {$s == ""} {
	continue;
    }

    # Looking for the wno header this way catches it even in the case
    # in which it is embedded in a header with a ccb (so it does 
    # not start a line).

    if {[regexp $wmoheader_regexp $line match]} {
	# split and get the wmo_time and wmo_station
	set wmo [split $match];
	set record(wmoid) [lindex $wmo 0];
	set record(wmostation) [lindex $wmo 1];
	set record(wmotime) [lindex $wmo 2];
	set record(wmocountry) [string range $record(wmoid) 2 3];

	continue;
    }

    # Records are saved with the flag of the keyword in the entire file,
    # or at the beginning of each record. 
    if {[regexp {^METAR} $s]} {
	set record(obtype) M;
    } elseif {[regexp {^SPECI} $s]} {
	set record(obtype)  S;
    }

    # Each line is handled according to how it starts, and whether
    # a record has not been closed (because it did not end with = so
    # we could not determine if it ended, until we read the next line).
    if {[match_mtrspecstart $s]} {
	# Start a new the record.  Strip the METAR or SPECI keyword.
	# If obdata_start sees the '=' sign it closes it.
	set s [string range $s 6 end];
	obdata_start $s;
    } elseif {[match_stationstart $s]} {
	# Same thing as above
	obdata_start $s;
    } elseif {[match_continuation $s] && ($record(_open) == 1)} {
	# A continuation line is anything that is ascii and which we get
	# when a record is still open.
	obdata_append $s;
    } elseif {$record(_open) == 1} {
	obdata_end;
    }
}

if {$record(_open) == 1} {
    obdata_end;
}

if {$fpath ne "-"} {
    close $f;
}

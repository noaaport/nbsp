#
# $Id$
#

proc proc_secstohm {secs} {

    return [clock format $secs -gmt true -format "%H:%M"]
}

proc proc_secstohms {secs} {

    return [clock format $secs -gmt true -format "%T"]
}

proc proc_fmtrow {n} {

    set fmt "<tr>"
    set i 1
    while {$i <= $n} {
	append fmt "<td>%s</td>"
	incr i
    }
    append fmt "</tr>\n"
    
    return $fmt
}

proc proc_fmtheader_chstats {n} {
#
# Format for the time, and two columns for each channel
#
    set fmt "<tr>"
    append fmt "<th>%s</th>"
    set i 1
    while {$i <= $n} {
	append fmt "<th colspan=2>%s</th>"
	incr i
    }
    append fmt "</tr>\n"
    
    return $fmt
}

proc proc_stringtoarray {str} {

    set strlist [split $str]
    set n [llength $strlist]
    set i 0
    while {$i < $n} {
	set j [expr $i + 1]
	set a($j) [lindex $strlist $i]
	set i $j
    }
    set a(0) $str

    return [array get a]
}

proc file_hasdata file {

    if {[file exists $file] && ([file size $file] != 0)} {
	return 1;
    }

    return 0;
}

proc nbsp_status {file logperiod_secs} {

    set fmt [proc_fmtrow 9]

    set result "<h3>Minute Summaries for products, frames, bytes</h3>\n";
    append result "<table border>\n";
    append result [format $fmt "time" \
		       "products<br>transmitted" "products<br>received" \
		       "products<br>retransmitted" "products<br>missed" \
		       "frames<br>received" "frames<br>jumps" \
		       "bytes<br>received" "kbytes/s"]

    set f [open "|tail $file" r]
    while {[gets $f finfo] > 0} {

	# Set first the default for hm and kbps and then check for
	# if the default was overriden

	array set a [proc_stringtoarray $finfo]
	set hm [proc_secstohm $a(1)]
	set kbps [expr int($a(6)/60000)]

	# We should ceck with  % 60 but we assume that the logperiod
	# is a multiple of 60 if it is more than 60.

	if {$logperiod_secs < 60} {
	    set hm [proc_secstohms $a(1)]
	    set kbps [expr int($a(6)/($logperiod_secs * 1000))]
	}

	append result [format $fmt $hm $a(7) $a(8) $a(10) $a(9) \
			   $a(2) $a(4) $a(6) $kbps]
    }
    close $f

    append result "</table>\n"

    return $result
}

proc nbsp_qstate {file logperiod_secs} {

    set fmt [proc_fmtrow 4]

    set result "<h3>Status of queues</h3>\n";
    append result "<table border>\n";
    append result [format $fmt "time" "processor" "filter" "server"]

    set f [open "|tail $file" r]
    while {[gets $f finfo] > 0} {

	# See the comment in the function nbsp_status about the
	# logperiod.

	array set a [proc_stringtoarray $finfo]
	set hm [proc_secstohm $a(1)]
	
	if {$logperiod_secs < 60} {
	    set hm [proc_secstohms $a(1)]
	}

	append result [format $fmt $hm $a(2) $a(3) $a(4)]
    }
    close $f

    append result "</table>\n"

    return $result
}

proc nbsp_missing file {

    set fmt [proc_fmtrow 2]

    set result "<h3>Products missed</h3>\n";
    append result "<table border>\n";
    append result [format $fmt "time" "name"]

    set f [open $file r]
    while {[gets $f finfo] > 0} {

	array set a [proc_stringtoarray $finfo]
	set hm [proc_secstohm $a(1)]

	append result [format $fmt $hm $a(7)]
    }
    close $f

    append result "</table>\n"

    return $result
}

proc nbsp_chstats_hour file {
#
# Outputs the table of produts received each minute in the hourly file.
#
    set fmt [proc_fmtrow 9]
    set fmtheader [proc_fmtheader_chstats 4]

    set result "<h3>Current number of products and bytes received per channel per minute</h3>\n";
    append result "<table border>\n";
    append result [format $fmtheader "time" "ch 1" "ch 2" "ch 3" "ch 4"]

    set f [open $file r]
    while {[gets $f finfo] > 0} {
	array set a [proc_stringtoarray $finfo]
	append result [format $fmt $a(1) $a(2) $a(6) $a(3) $a(7) \
			   $a(4) $a(8) $a(5) $a(9)]
    }
    close $f

    append result "</table>\n"

    return $result
}

proc nbsp_chstats_hour_summary file {
#
# This outputs a one-line summary of the hourly file. It is used by
# nbsp_chstats_day.

    set fmt [proc_fmtrow 9]

    set files(0) 0
    set files(1) 0
    set files(2) 0
    set files(3) 0
    set bytes(0) 0
    set bytes(1) 0
    set bytes(2) 0
    set bytes(3) 0

    set f [open $file "r"]
    while {[gets $f stats] > 0} {
        array set a [proc_stringtoarray $stats];
        set files(0) [expr $files(0) + $a(2)];
        set files(1) [expr $files(1) + $a(3)];
        set files(2) [expr $files(2) + $a(4)];
        set files(3) [expr $files(3) + $a(5)];
        set bytes(0) [expr $bytes(0) + $a(6)];
        set bytes(1) [expr $bytes(1) + $a(7)];
        set bytes(2) [expr $bytes(2) + $a(8)];
        set bytes(3) [expr $bytes(3) + $a(9)];
    }
    close $f;

    set hh [string range $a(1) 0 1]
    set result [format $fmt $hh $files(0) $bytes(0) $files(1) $bytes(1) \
		       $files(2) $bytes(2) $files(3) $bytes(3)]

    return $result
}

proc nbsp_chstats_day filelist {
#
# This outputs a summary of the hourly files for the current day.
#
    set fmt [proc_fmtrow 5];
    set fmtheader [proc_fmtheader_chstats 4];
    array set statsfilelist $filelist;

    set result "<h3>Number of products and bytes received per channel since midnight</h3>\n";
    append result "<table border>\n";
    append result [format $fmtheader "hour" "ch 1" "ch 2" "ch 3" "ch 4"]

    foreach hh [lsort [array names statsfilelist]] {
	append result [nbsp_chstats_hour_summary $statsfilelist($hh)];
    }

    append result "</table>\n";

    return $result;
}

proc nbsp_connections file {

    set fmt [proc_fmtrow 4];

    set table "<table border>\n";
    append table [format $fmt "ip" "type" "queue" "time"];

    set numconn 0;
    set f [open $file r];
    while {[gets $f finfo] > 0} {

	if {[regexp {^(-|\s*$)} $finfo]} {
	    continue;
	}

	array set a [proc_stringtoarray [string trimleft $finfo]];
	set time [clock format $a(4) -format "%Y%m%d %H:%M"];
	append table [format $fmt $a(1) $a(2) $a(3) $time];

	incr numconn;
    }
    close $f;

    append table "</table>\n";

    set result "<h3>Active connections - $numconn</h3>\n";
    if {$numconn != 0} {
        append result $table;
    }
    return $result;
}

proc nbsp_slavestats file {

    set fmt [proc_fmtrow 10]

    set table "<table border>\n";
    append table [format $fmt "masterhost:port" "time" "conn errs" \
		      "conn time" "errors" "packets" "bytes" \
		      "reset time" "errors" "packets" "bytes"];

    array set data {};
    set f [open $file r];
    while {[gets $f finfo] > 0} {
	set finfo_parts [split $finfo];
	set masterhostport [lindex $finfo_parts 0];
	set data(${masterhostport}) $finfo;
    }
    close $f;

    foreach key [array names data] {
	set finfo $data($key);
	array set a [proc_stringtoarray [string trimleft $finfo]]
	set time [clock format $a(2) -format "%Y%m%d %H:%M"]
	set ctime [clock format $a(4) -format "%Y%m%d %H:%M"]
	set rtime [clock format $a(8) -format "%Y%m%d %H:%M"]

	append table [format $fmt $key $time $a(3) \
			  $ctime $a(5) $a(6) $a(7) \
			  $rtime $a(9) $a(10) $a(11)];
    }

    append table "</table>\n"

    set result "<h3>Slave connections</h3>\n";
    append result $table;

    return $result
}

proc display_statplots {type htdocsdir statplotsubdir nbspdstatusfile} {

    set basedir [file join $htdocsdir $statplotsubdir];

    switch -exact $type {
	0 {set _opt_h ""; append result "<h3>Hourly summary</h3>\n"}
	1 {set _opt_h "-m -h -0"; append result "<h3>Current hour</h3>\n"}
	2 {set _opt_h "-m -h -1"; append result "<h3>Last hour</h3>\n"}
	3 {set _opt_h "-m -h -2"; append result "<h3>Last two hours</h3>\n"}
	4 {set _opt_h "-m"; append result "<h3>Since midnight</h3>\n"}
    }
    set status [catch {
	eval exec nbspstatplot -b $basedir -s ${_opt_h} $nbspdstatusfile;
    } errmsg];

    if {$status != 0} {
	return "";
    }

    # Create the code for the output page
    set baseurl [file join / $statplotsubdir];
    append result "<img src=\"[file join $baseurl mbytes.png]\">\n";
    append result "<img src=\"[file join $baseurl ftrans.png]\">\n";
    append result "<img src=\"[file join $baseurl fretrans.png]\">\n";
    append result "<img src=\"[file join $baseurl fmiss.png]\">\n";
    append result "<img src=\"[file join $baseurl framesrcv.png]\">\n";
    append result "<img src=\"[file join $baseurl framesjumps.png]\">\n";

    return $result;
}

proc display_config {} {

    set fmt [proc_fmtrow 2];

    append result "<table border>\n";

    append result [format $fmt "Package version" [exec nbspversion]];
    foreach line [split [exec nbspd -C 2> /dev/null] "\n"] {
	set parts [split $line];
        set p [lindex $parts 0];
        set v [join [lreplace $parts 0 0] " "];
        append result [format $fmt $p $v];
    }

    append result "</table>\n";
}

proc nbsp_received file {

    set fmt [proc_fmtrow 3]

    set hhmm [file rootname [file tail $file]]

    set result "<h3>Products received at $hhmm</h3>\n"
    append result "<table border>\n"
    append result [format $fmt "time" "size" "name"]

    set f [open $file r]
    while {[gets $f finfo] > 0} {
	array set a [proc_stringtoarray $finfo]
	set hms [proc_secstohms $a(1)]
	#
	# The path will be output with a link to the file in the /spool
	# domain handler. a(8) is the fname. The proc nbsp_get_file_exists
	# is defined in nbspget.tcl.
	#
	if {[nbsp_get_file_exists $a(8)]} {
	    set pathlink "<a href=\"/_get/spool/$a(8)\">$a(8)</a>";
	} else {
	    set pathlink $a(8);
	}
	append result [format $fmt $hms $a(7) $pathlink]
    }
    close $f

    append result "</table>\n"

    return $result
}

proc nbsp_received_hour {hh {mm 59}} {
#
# The mm argument, if given, determines the maximum minute to include. It is
# used when the function is called for the current hour.
#
    global Config;

    set fulllist [lsort [glob -tails -nocomplain \
			     -directory $Config(nbspinvdir) \
			     ${hh}*$Config(nbspinvfext)]];

    if {$mm eq "59"} {
	set flist $fulllist;
    } else {
	set flist [list];
	set max_hhmm ${hh}${mm};
	foreach file $fulllist {
	    lappend flist $file;
	    set hhmm [file rootname $file];
	    if {$hhmm eq $max_hhmm} {
		break;
	    }
	}
    }

    if {[llength $flist] == 0} {
	set result "<h5>No products received at $hh.</h5>\n";
	return $result;
    }

    set result "<h3>Products received at $hh</h3>\n"; 
    foreach file $flist {
	set hhmm [file rootname $file];
	set href "<a href=\"received_minute?hhmm=$hhmm\">$hhmm</a>\n";
	append result $href;
    }

    return $result;
}

proc nbsp_received_last_Nhours {N} {
#
# List of products received in the last N (e.g. 4) hours.
#
    set now [clock seconds];
    set hh_now [clock format $now -gmt true -format "%H"];

    # Current hour
    set hh [clock format $now -format "%H" -gmt true];
    set mm [clock format $now -format "%M" -gmt true];
    set result [nbsp_received_hour $hh $mm];

    set t $now;
    set i 1;
    while {$i < $N} {
	incr t -3600;
	set hh [clock format $t -gmt true -format "%H"];
	append result [nbsp_received_hour $hh];

	incr i;
    }

    return $result;
}

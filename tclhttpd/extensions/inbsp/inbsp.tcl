#
# $Id$
#
# Functions to support the iNbsp Stats Project
#
# Usage: http://<server>[:<port>]/<command>
#
# where <command> is
#
# /_inbsp/stats[?format=std|yaml|xml|csv|csvk]     (status file)
# /_inbsp/missing[?format=xml|csv]   (missing file: unimplemented)
# /_inbsp/<xxx>[?format=xml|csv]     (other file: unimplemented)
# /_inbsp/monitor  (inventory filter file <hh>.stats)
#
# All data is retrieved via the inventory filter tool nbspstatcounters
#
Direct_Url /_inbsp _inbsp;

set inbsp(conf) "inbsp.conf";
set inbsp(confdir) $Config(confdir);
set inbsp(localconfdirs) $Config(localconfdirs);

# Non-configurable
set inbsp(data_type) "_inbsp/stats";

# The local overrides
set _inbspconf [file join $inbsp(confdir) $inbsp(conf)];
if {[file exists ${_inbspconf}]} {
    source ${_inbspconf};
}
unset _inbspconf;

proc _inbsp/stats {{format "std"}} {

    global inbsp;

    # The mechanism used to return content types is to set a global
    # variable with the same name as this function.
    set _inbsp/stats "text/plain";

    if {$format eq "xml"} {
	set _inbsp/stats "text/xml";
	set r "[inbsp_output_xml_start $inbsp(data_type)]\n";
    } else {
	set r "";
    }

    append r [inbsp_output_stats $format];

    if {$format eq "xml"} {
	append r "\n[inbsp_output_xml_end $inbsp(data_type)]";
    }

    return $r;
}

proc inbsp_output_stats {format} {

    global inbsp;

    set status [catch {
	set data [exec nbspstatcounters -f $format];
    } errmsg];

    if {$status != 0} {
	return "";
    }

    # Prepend the data_type and any other metadata before the data
    # output by npemwinstatcounters (unless the requested format is "stdh"
    # which contains the header from nbspstatcounters).
    set r "";
    foreach k [list data_type] {
	if {$format eq "std"} {
	    append r "$k=$inbsp($k)\n";
	} elseif {$format eq "csv"} {
	    append r "$inbsp($k),";
	} elseif {$format eq "csvk"} {
	    append r "$k=$inbsp($k),";
	} elseif {$format eq "xml"} {
	    set r "<$k>$inbsp($k)</$k>\n";
	}
    }

    append r [string trim $data];

    return $r;
}

proc inbsp_output_xml_start {type} {

    set r "<?${type} version=\"1.0\"?>";

    return $r;
}

proc inbsp_output_xml_end {type} {

    set r "</${type}>";

    return $r;
}

proc _inbsp/monitor {} {

    # The mechanism used to return content types is to set a global
    # variable with the same name as this function.
    set _inbsp/summary "text/plain";
    
    # error codes
    set err_stats_time 1;
    set err_chstats_time 2;
    set err_chstats_data 3;

    set total_files 0;
    set total_bytes 0;

    set status [catch {
	set rawoutput [exec nbspstatcounters -f "csvk"];
    } errmsg];

    if {$status != 0} {
	return "";
    }

    set output [split $rawoutput ","];

    foreach entry $output {
	if {[regexp {chstats_files} $entry]} {
	    set val [string trim [lindex [split $entry "="] 1]];
	    incr total_files $val;
	}

	if {[regexp {chstats_bytes} $entry]} {
	    set val [string trim [lindex [split $entry "="] 1]];
	    incr total_bytes $val;
	}

	if {[regexp {chstats_time} $entry]} {
	    set chstats_time [string trim [lindex [split $entry "="] 1]];
	}

	if {[regexp {^stats_time} $entry]} {	             
	    set stats_time [string trim [lindex [split $entry "="] 1]];
	}
    }

    set seconds [clock seconds];
 
    # make the test
    set code 0;

    set stats_timediff [expr $seconds - $stats_time];
    if {[expr $stats_timediff > 60]} {
	set code $err_stats_time;
    }

    set chstats_timediff [expr $seconds - $chstats_time];
    if {[expr $chstats_timediff > 60]} {
	set code $err_chstats_time;
    }

    if {($total_files == 0) || ($total_bytes == 0)} {
	set code $err_chstats_data;
    }

    set rlist [list $seconds $stats_time $chstats_time \
		   $stats_timediff $chstats_timediff \
		   $total_files $total_bytes];
    set r [join $rlist " "];
    append r "\n";

    return [join [list "|monitorcode=$code" $r] "|"];
}

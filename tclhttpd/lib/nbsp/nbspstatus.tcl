#
# $Id$
#

#
## Direct_Url /nbsp nbsp
#

package require html

proc nbsp/status/summary {} {

    global Config

    set nbsp_status_file $Config(nbspstatusfile)
    set nbsp_qstate_file $Config(nbspqstatelogfile)
    set nbsp_stats_awk $Config(nbsplibdir)/nbsp_stats.awk
    set nbspstats_logperiod_secs $Config(nbspstats_logperiod_secs)
    set qstate_logperiod_secs $Config(qstate_logperiod_secs)

    if {[file_hasdata $nbsp_status_file] == 0} {
	return "$nbsp_status_file not found or empty."
    }

    if {[file_hasdata $nbsp_qstate_file] == 0} {
	return "$nbsp_qstate_file not found or empty."
    }

    set status [catch {
	set result [exec awk -f $nbsp_stats_awk $nbsp_status_file]
    } errmsg]
    if {$status != 0} {
	set result "";
    }

    append result [nbsp_status $nbsp_status_file $nbspstats_logperiod_secs]
    append result [nbsp_qstate $nbsp_qstate_file $qstate_logperiod_secs]

    return $result
}

proc nbsp/status/stats {} {
#
# Daily Statistics summary of products and frames received and missed
# and minute summary for the last ten minutes.
#
    global Config

    set nbsp_status_file $Config(nbspstatusfile)
    set nbsp_stats_awk $Config(nbsplibdir)/nbsp_stats.awk
    set nbspstats_logperiod_secs $Config(nbspstats_logperiod_secs)

    if {[file_hasdata $nbsp_status_file] == 0} {
	return "$nbsp_status_file not found or empty."
    }

    set status [catch {
	set result [exec awk -f $nbsp_stats_awk $nbsp_status_file]
    } errmsg]
    if {$status != 0} {
	set result "";
    }

    append result [nbsp_status $nbsp_status_file $nbspstats_logperiod_secs]

    return $result
}

proc nbsp/status/qstate {} {
#
# State of the queues
#
    global Config

    set nbsp_qstate_file $Config(nbspqstatelogfile)
    set qstate_logperiod_secs $Config(qstate_logperiod_secs)

    if {[file_hasdata $nbsp_qstate_file] == 0} {
	return "$nbsp_qstate_file not found or empty."
    }

    set result [nbsp_qstate $nbsp_qstate_file $qstate_logperiod_secs]

    return $result
}

proc nbsp/status/qdbstats {} {
#
# qdbstats
#
    global Config;

    set nbsp_qdbstats_file $Config(nbspqdbstatslogfile);

    if {[file_hasdata $nbsp_qdbstats_file] == 0} {
	return "$nbsp_qdbstats_file not found or empty.";
    }
    
    set result "<pre>";
    set f [open $nbsp_qdbstats_file r];
    append result [read $f];
    close $f;
    append result "</pre>";

    return $result;
}

proc nbsp/status/mdbstats {} {
#
# mspool dbstats
#
    global Config;

    set nbsp_mdbstats_file $Config(nbspmspoolbdb_dbstats_logfile);

    if {[file_hasdata $nbsp_mdbstats_file] == 0} {
	return "$nbsp_mdbstats_file not found (memory spool not configured?)."
    }
    
    set result "<pre>";
    set f [open $nbsp_mdbstats_file r];
    append result [read $f];
    close $f;
    append result "</pre>";

    return $result
}

proc nbsp/status/missing {} {
#
# List of missing products since midnight
#
    global Config

    set nbsp_missing_file $Config(nbspmissinglogfile)

    if {[file_hasdata $nbsp_missing_file] == 0} {
	return "$nbsp_missing_file not found or empty."
    }
    
    return [nbsp_missing $nbsp_missing_file]
}

proc nbsp/status/chstats {} {
#
# Per-channel statistics.
#
    global Config

    set seconds [clock seconds]
    set hh [clock format $seconds -gmt true -format "%H"]
    set hour [clock format $seconds -gmt true -format "%k"]

    set nbsp_chstats_file $Config(nbspinvdir)/${hh}.stats
    
    if {[file_hasdata $nbsp_chstats_file] == 0} {
	return "$nbsp_chstats_file not found or empty."
    }

    # This outputs the satistics for each minute in the current hour.
    set result [nbsp_chstats_hour $nbsp_chstats_file]

    # Now the table of all the hour sats files, since midnight until
    # the current hour. Build the file list that will be pased to the function.
    
    set h 0
    while {$h <= $hour} {
        if {$h <= 9} {
            set hh "0$h"
        } else {
            set hh $h
        }
	if {[file_hasdata $Config(nbspinvdir)/${hh}.stats]} {
	    set nbsp_chstats_filelist($hh) $Config(nbspinvdir)/${hh}.stats
	}
	incr h
    }

    # Pass to the function the list of alternates between indices and values
    append result [nbsp_chstats_day [array get nbsp_chstats_filelist]]

    return $result
}

proc nbsp/status/connections {} {
#
# Active connections
#
    global Config

    set nbsp_active_file $Config(nbspserveractivefile);

    if {[file_hasdata $nbsp_active_file] == 0} {
	return "$nbsp_active_file not found or empty.";
    }

    return [nbsp_connections $nbsp_active_file];
}

proc nbsp/status/slavestats {} {
#
# The status/stats of connections of the slave threads.
#
    global Config

    set nbsp_slavestats_file $Config(nbspslavestatsfile);

    if {[file_hasdata $nbsp_slavestats_file] == 0} {
	return "$nbsp_slavestats_file not found or empty.";
    }

    return [nbsp_slavestats $nbsp_slavestats_file];
}

proc nbsp/status/statplot {type} {
#
# Statistics plots.
#
    global Config

    ::html::init;

    if {$type > 0} {
	append result [::html::refresh 60] "\n";
    }
    append result [::html::head "Statistics summary graphs"] "\n";
    append result [::html::bodyTag] "\n";

    append result "<h1>Statistics summary graphs</h1>\n";
    append result [display_statplots $type $Config(docRoot) \
		       $Config(nbspstatplothtdir) $Config(nbspstatusfile)];

    append result [::html::end];

    return $result;
}

proc nbsp/status/printconf {} {
#
# Print current nbspd configuration.
#

    set result "<h1>Configuration parameters</h1>\n";

    append result [display_config];

    return $result;
}

proc nbsp/status/received_last_minute {} {
#
# List of received products in the last minute
#
    global Config

    set now [clock seconds]
    set last [clock format [expr $now - 65] -format "%H%M" -gmt true]
    
    set nbsp_received_file $Config(nbspinvdir)/${last}$Config(nbspinvfext)
    
    if {[file_hasdata $nbsp_received_file] == 0} {
	return "$nbsp_received_file not found or empty."
    }

    return [nbsp_received $nbsp_received_file]
}

proc nbsp/status/received_minute {hhmm} {
#
# List of received products in a given minute.
#
    global Config

    set nbsp_received_file $Config(nbspinvdir)/${hhmm}$Config(nbspinvfext)
    
    if {[file_hasdata $nbsp_received_file] == 0} {
	return "$nbsp_received_file not found or empty."
    }

    return [nbsp_received $nbsp_received_file]
}

proc nbsp/status/received_past_hour {} {
#
# List of products received in the past hour
#
    set now [clock seconds]
    set t [expr $now - 3600]
    set hh [clock format $t -format "%H" -gmt true]

    return [nbsp_received_hour $hh]
}

proc nbsp/status/received_last_hour {} {
#
# List of products received within the last hour
#
    set now [clock seconds]
    set hh [clock format $now -format "%H" -gmt true]
    set mm [clock format $now -format "%M" -gmt true]

    return [nbsp_received_hour $hh $mm]
}

proc nbsp/status/received_last_24hours {} {
#
# List of products received in the last 24 hours.
#
    set now [clock seconds]
    set hh_now [clock format $now -gmt true -format "%H"]

    # Current hour
    set hh [clock format $now -format "%H" -gmt true]
    set mm [clock format $now -format "%M" -gmt true]	
    set result [nbsp_received_hour $hh $mm]   

    set t $now;
    set done 0;
    while {$done == 0} {
	incr t -3600;
	set hh [clock format $t -gmt true -format "%H"]; 
	if {$hh eq $hh_now} {
	    set done 1;
	} else {
	    append result [nbsp_received_hour $hh]
	}
    }

    return $result;
}

proc nbsp/status/received_last_3hours {} {
#
# List of products received in the last 3 hours.
#
    return [nbsp_received_last_Nhours 3];
}

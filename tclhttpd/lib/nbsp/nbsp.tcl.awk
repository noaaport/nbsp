Direct_Url /nbsp nbsp

# At this moment [Sun Feb 11 10:36:14 AST 2007] the only awk
# script used in nbsp.tcl is nbsp_stats.awk. The functionality
# of the others is implemented directly in the functions
# in functions.tcl. This file is the implementation using
# the awk scripts, which I decided not to use anymore (with
# the exception mentioned above).

proc nbsp {} {

    global Config

    set nbsp_stats_file $Config(nbspstatsdir)/nbspd.status
    set nbsp_qstate_file $Config(nbspstatsdir)/nbspd.qstate
    set nbsp_stats_awk $Config(nbsplibdir)/nbsp_stats.awk 
    set nbsp_status_awk $Config(nbsplibdir)/nbsp_status.awk
    set nbsp_qstate_awk $Config(nbsplibdir)/nbsp_qstate.awk

#    set result [exec cat $Config(nbsphtincludedir)/header.html]
    append result [exec awk -f $nbsp_stats_awk $nbsp_stats_file]
    append result [exec tail $nbsp_stats_file | awk -f $nbsp_status_awk]
    append result [exec tail $nbsp_qstate_file | awk -f $nbsp_qstate_awk]
#    append result [exec cat $Config(nbsphtincludedir)/footer.html]

    return $result
}

proc nbsp/stats {} {
#
# Daily Statistics summary of products and frames received and missed
# and minute summary for the last ten minutes.
#
    global Config

    set nbsp_stats_file $Config(nbspstatsdir)/nbspd.status
    set nbsp_stats_awk $Config(nbsplibdir)/nbsp_stats.awk 
    set nbsp_status_awk $Config(nbsplibdir)/nbsp_status.awk

    set result [exec awk -f  $nbsp_stats_awk $nbsp_stats_file]
    append result [exec tail $nbsp_stats_file | awk -f $nbsp_status_awk]

    return $result
}

proc nbsp/qstate {} {
#
# State of the queues
#
    global Config

    set nbsp_qstate_file $Config(nbspstatsdir)/nbspd.qstate
    set nbsp_qstate_awk $Config(nbsplibdir)/nbsp_qstate.awk

    set result [exec tail $nbsp_qstate_file | awk -f $nbsp_qstate_awk]

    return $result
}

proc nbsp/missing {} {
#
# List of missing products since midnight
#
    global Config

    set nbsp_missing_file $Config(nbspstatsdir)/nbspd.missing
    set nbsp_missing_awk $Config(nbsplibdir)/nbsp_missing.awk
    
    return [exec awk -f $nbsp_missing_awk $nbsp_missing_file]
}

proc nbsp/received {} {
#
# List of received products in the last minute
#
    global Config

    set now [clock seconds]
    set last [clock format [expr $now - 60] -format "%H%M" -gmt true]
    
    set nbsp_received_file $Config(nbspinvdir)/${last}.log
    set nbsp_received_awk $Config(nbsplibdir)/nbsp_received.awk
    
    if {[file exists $nbsp_received_file] == 0} {
	return "$nbsp_received_file not found."
    }

    return [exec awk -f $nbsp_received_awk $nbsp_received_file]
}

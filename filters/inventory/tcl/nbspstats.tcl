#!%TCLSH%
#
# $Id$
#
# Usage: nbspstats [-l <statsawkscript>] [-p <logperiod>] [-q <statefile>]
#        [-s <statusfile>] [stats|qstate]
#
# If no command is given, "stats" is assumed. The -l option can be used
# to specify an awk script for the "stats" report. The logperiod in -p
# is used only for the stats summary (to calculate the lbytes/s).
#
package require cmdline;

set usage {nbspstats [-l <statsawkscript>] [-p <logperiod>]
           [-q <statefile>] [-s <statusfile>] [stats|qstate]};
set optlist {{l.arg ""} {p.arg 60} {q.arg ""} {s.arg ""}};

# functions
proc proc_nbspstats_qstate {} {

    global nbspstats;

    set r [list];
    lappend r {     date-time    seconds       pq       fq       sq};

    set data [split [string trim [exec tail $nbspstats(qstatefile)]] "\n"];
    foreach line $data {
	set line_parts [split $line];
	set seconds [lindex $line_parts 0];
	set pq [lindex $line_parts 1];
	set fq [lindex $line_parts 2];
	set sq [lindex $line_parts 3];
	set datetime [clock format $seconds -format "%Y%m%d-%H:%M"];
	lappend r [format "%25s %8s %8s %8s" "$datetime $seconds" \
		       $pq $fq $sq];
    }
    return [join $r "\n"];
}

proc proc_set_stats_awk_script {} {

    global option;

    if {$option(l) ne ""} {
	return [exec cat $option(l)];
    }

    # The "logperiod" is passed as a variable with -v

    return {
	BEGIN {
	  prod_total = 0;
	  prod_received = 0;
	  prod_missed = 0;
	  prod_rtx = 0;
	  prod_rtx_c = 0;
	  prod_rtx_p = 0;
	  prod_rtx_i = 0;
	  prod_recovered = 0;
	  frames_jumps = 0;
	  frames_received = 0;
	  bytes_received = 0;

	  fmt = "%20s\t%.3e\t%.3e\t%d\n";
	  fmt1 = "%20s\t%.3e\t%.3e\t%d (%d KB/s)\n";
	  fmt2 = "%20s\t%f\n";
	}

	END {

	  prod_missed_fraction = prod_missed/prod_total;
	  prod_rtx_fraction = prod_rtx/prod_total;

	  printf("%29s\t\t%s\t%s\n", "total", "per minute", "last minute");
	  printf(fmt, "products trasmitted", prod_total, prod_total/NR,
		 last_prod_transmitted);
	  printf(fmt, "products received", prod_received, prod_received/NR,
		 last_prod_received);
	  printf(fmt, "products missed", prod_missed, prod_missed/NR,
		 last_prod_missed);
	  printf(fmt, "products rtx", prod_rtx, prod_rtx/NR, last_prod_rtx);
	  printf(fmt, "products rtx comp", prod_rtx_c, prod_rtx_c/NR,
		 last_prod_rtx_c);
	  printf(fmt, "products rtx proc", prod_rtx_p, prod_rtx_p/NR,
		 last_prod_rtx_p);
	  printf(fmt, "products rtx ign", prod_rtx_i, prod_rtx_i/NR,
		 last_prod_rtx_i);
	  printf(fmt, "frames received", frames_received, frames_received/NR,
		 last_frames_received);
	  printf(fmt, "frames jumps", frames_jumps, frames_jumps/NR,
		 last_frames_jumps);
	  printf(fmt1, "bytes received", bytes_received, bytes_received/NR,
		 last_bytes_received, last_bytes_received/(logperiod*1000));
	  printf(fmt2, "fraction missing", prod_missed_fraction);
	}

	{
	  prod_total += $7;
	  prod_received += $8;
	  prod_missed += $9;
	  prod_rtx += $10;
	  prod_rtx_c += $11;
	  prod_rtx_p += $12;
	  prod_rtx_i += $13;
	  prod_recovered += $14;
	  frames_received += $2;
	  frames_jumps += $4;
	  bytes_received += $6;

	  last_prod_transmitted = $7;
	  last_prod_received = $8;
	  last_prod_missed = $9;
	  last_prod_rtx = $10;
	  last_prod_rtx_c = $11;
	  last_prod_rtx_p = $12;
	  last_prod_rtx_i = $13;
	  last_prod_recovered = $14;
	  last_frames_received = $2;
	  last_frames_jumps = $4;
	  last_bytes_received = $6;
	}
    }
}

## The common defaults
set _defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists ${_defaultsfile}] == 0} {
    puts "${_defaultsfile} not found.";
    return 1;
}
source ${_defaultsfile};
unset _defaultsfile;

# nbspd.init is needed for nbspd.status
set nbspd_init_file [file join $common(libdir) nbspd.init];
if {[file exists $nbspd_init_file] == 0} {
    puts "$nbspd_init_file not found.";
    return 1;
}
source $nbspd_init_file;
unset nbspd_init_file;

set nbspstats(qstatefile) $nbspd(qstatelogfile);
set nbspstats(statusfile) $nbspd(statusfile);

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

# The default log period is 60 seconds but that can be changed with the
# -p option.
set logperiod $option(p);

if {$option(q) ne ""} {
    set nbspstats(qstatefile) $option(q);
}

if {$option(s) ne ""} {
    set nbspstats(statusfile) $option(s);
}

set stats_awk_script [proc_set_stats_awk_script];
#
set nbspstats(qstate,proc) "proc_nbspstats_qstate";
set nbspstats(stats,cmd) [list awk -v logperiod=$logperiod \
			      $stats_awk_script \
			      $nbspstats(statusfile)];

if {$argc > 1} {
    puts $usage;
    return 1;
} elseif {$argc == 1} {
    set cmd [lindex $argv 0];
} else {
    set cmd "stats";	# the default if none is given
}

if {[info exists nbspstats($cmd,cmd)]} {
    set r [eval exec $nbspstats($cmd,cmd)];
    puts $r;
} elseif {[info exists nbspstats($cmd,proc)]} {
    set r [eval $nbspstats($cmd,proc)];
    puts $r;
} else {
    puts "$cmd command not configured.";
    return 1;
}

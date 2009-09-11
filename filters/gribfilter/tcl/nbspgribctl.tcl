#!%TCLSH%
#
# $Id$
#
# Usage: nbspgribctl [-c ctlfile] [-i idxfile] [-p <pdeffile>] <gribfile>
#
# If <ctlfile>, <idxfile> <pdeffile> are existing directories, then the
# ctl and idx files are placed there with the default names.
#
package require cmdline;

set usage {Usage: nbspgribctl [-c ctlfile] [-i idxfile]
    [-p pdeffile] <gribfile>};
set optlist {{c.arg ""} {i.arg ""} {p.arg ""}};

array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];
if {$argc != 1} {
    puts "Error: $usage";
    exit 1;
}
set grads(grbfile) [lindex $argv 0];

# First set the defaults, then check the cmd-line overrides
set grads(ctlfile) "[file rootname [file tail $grads(grbfile)]].ctl";
set grads(idxfile) "[file rootname [file tail $grads(grbfile)]].idx";
set grads(pdeffile) "[file rootname [file tail $grads(grbfile)]].pdef";

if {$option(c) ne ""} {
    if {[file isdirectory $option(c)]} {
	set grads(ctlfile) [file join $option(c) $grads(ctlfile)];
    } else {
	set grads(ctlfile) $option(c);
    }
}

if {$option(i) ne ""} {
    if {[file isdirectory $option(i)]} {
	set grads(idxfile) [file join $option(i) $grads(idxfile)];
    } else {
	set grads(idxfile) $option(i);
    }
}

if {$option(p) ne ""} {
    if {[file isdirectory $option(p)]} {
	set grads(pdeffile) [file join $option(p) $grads(pdeffile)];
    } else {
	set grads(pdeffile) $option(p);
    }
}

# grib2ctl sometimes emits an informational message. We will flag it as error
# only if the ctl file cannot be produced.
set msg "";
set status [catch {
    set msg [exec grib2ctl -verf $grads(grbfile) \
		 $grads(idxfile) $grads(pdeffile) > $grads(ctlfile)];
} errmsg];

if {[file exists $grads(ctlfile)] == 0} {
    puts "grib2ctl could not create $grads(ctlfile)";
    if {$msg ne ""} {
	puts $msg;
    }
    exit 1;
}

# Work around bug in gribmap (does not set exit status correctly)
set msg "";
set status [catch {		 
    set msg [exec gribmap -q -i $grads(ctlfile)];
} errmsg];

if {[file exists $grads(idxfile)] == 0} {
    puts "gribmap could not create $grads(idxfile).";
    if {$msg ne ""} {
	puts $msg;
	exit 1;
    }
}

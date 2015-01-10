#!%TCLSH%
#
# $Id$
#
# Usage: nbspgribctl [-e] [-c ctlfile] [-i idxfile] [-p <pdeffile>] <gribfile>
#
# If <ctlfile>, <idxfile> <pdeffile> are existing directories, then the
# ctl and idx files are placed there with the default names.
#
# The default is to process grib1. If the file ends with grb2 or grib2, or
# if "-e" is given, then it processes it as grib2.
#
package require cmdline;

set usage {Usage: nbspgribctl [-e] [-c ctlfile] [-i idxfile]
    [-p pdeffile] <gribfile>};
set optlist {e {c.arg ""} {i.arg ""} {p.arg ""}};

array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];
if {$argc != 1} {
    puts "Error: $usage";
    exit 1;
}
set grads(grbfile) [lindex $argv 0];

# First set the defaults, then check the cmd-line overrides
set _rootname [file rootname [file tail $grads(grbfile)]];
append grads(ctlfile) ${_rootname} ".ctl";
append grads(idxfile) ${_rootname} ".idx";
append grads(pdeffile) ${_rootname} ".pdef";

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

# Get the grib edition from the extension of the -e switch
set _ext [file extension [file tail $grads(grbfile)]];
if {[regexp {gri?b2} ${_ext}] || ($option(e) == 1)} {
    set grib2ctl_cmd "g2ctl";
} else {
    set grib2ctl_cmd "grib2ctl -verf";
}

# In case grib2ctl emits an informational message, we will flag it as error
# only if the ctl file cannot be produced.
set msg "";
set status [catch {
    set msg [eval exec $grib2ctl_cmd $grads(grbfile) \
		 $grads(idxfile) $grads(pdeffile) > $grads(ctlfile)];
} errmsg];

if {[file exists $grads(ctlfile)] == 0} {
    puts "grib2ctl (or g2ctl) could not create $grads(ctlfile)";
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

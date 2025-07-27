#!%TCLSH%
#
# This script takes as argument the same kind of file as "craftinsert".
# The intention here is to process the file in a way similar to craftinsert,
# but taking the configurable options from the cmd line instead
# of a configuration file as is the case with craftinsert.
# For the moment it simply calls craftinsert.
# NOTE: Remember that craftinsert uses the "umask" function which is in tclx.
#

proc msg {s} {

    global g;

    if {$g(verbose) == 0} {
	return;
    }

    if {$g(background) == 0} {
	puts stdout "$g(name): $s";
    } else {
	exec logger -t $g(name) $s;
    }
}

proc err {s} {
    
    global g;

    if {$g(background) == 0} {
	puts stderr "$g(name): $s";
    } else {
	exec logger -t $g(name) $s;
    }
}

# globals
set g(name) [file tail $argv0];

# options
set g(background) 1;
set g(verbose) 0;

# path to craftinsert
set g(craftinsert) "%CRAFTINSERT_LIBDIR%/craftinsert";

# variables
set status 0;

#
# main
#
set argc [llength $argv];
if {$argc == 0} {
    err "Requires an argument.";
}

# Get the path to the input file
# (called ppath in the craffilter terminology)
#
set ppath [lindex $argv 0];

set status [catch {
    exec $g(craftinsert) $ppath;
} errmsg];

if {$status != 0} {
    err $errmsg;
}

return $status;

#!%TCLSH%
#
# $Id$
#
# usage: nbsppanic [-b] [-p <pause_msecs>] [-r]
#
# This script tries to bring the nbsp scratch areas to a clean state:
#
#   (1) Deletes all the db-related files in the db directory.
#   (2) Deletes all files in /var/run/nbsp
#
# It ensures that nbspd is not running by sending a ``pkill -KILL nbspd''
# before doing anything.
#
# -b => run in the background (report errors to syslog instead of stderr)
# -p => pause miliseconds between stages (default is 2 secs)
# -r => try to restart nbspd after doing the cleanup
#
set usage {nbsppanic [-b] [-p <pause_secs>] [-r]};
set optlist {b {p.arg ""} r};

# Let tcl report the errors if these files do not exist. nbspd.init defines,
# via the nbspd(), the nbspd settings that we need, e.g.,
# the run and bdb directories.
#
source "/usr/local/etc/nbsp/filters.conf";
source "/usr/local/libexec/nbsp/nbspd.init";

# Packages from tcllib
package require cmdline;

# Nbsp packages
## The errx library. syslog enabled below if -b is given.
package require nbsp::errx;

# configuration
set nbsppanic(pause_msecs) 2000;
set nbsppanic(rcfpath) "%RCFPATH%";

#
# init
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$option(b) == 1} {
    ::nbsp::syslog::usesyslog
}

# Don't set it less than the default
if {$option(p) > $nbsppanic(pause_msecs)} {
    set nbsppanic(pause_msecs) $option(p);
}

#
# main
#
after $nbsppanic(pause_msecs);

catch {exec pkill -KILL "nbspd"};
after $nbsppanic(pause_msecs);

# Clean the run directory, if it exists.
if {[file isdirectory $nbspd(rundir)]} {
    cd $nbspd(rundir);
    foreach f [glob -directory "." -tails -nocomplain "*"] {
	file delete $f;
    }
    after $nbsppanic(pause_msecs);
}

# bdb dir
cd $nbspd(bdbdir);
foreach f [glob -directory "." -tails -nocomplain "__db*"] {
    file delete $f;
}

foreach f [glob -directory "." -tails -nocomplain "q*"] {
    file delete $f;
}

foreach f [glob -directory "." -tails -nocomplain "pctl*"] {
    file delete $f;
}
after $nbsppanic(pause_msecs);

# If -r was given try to restart it
if {$option(r) == 1} {
    exec $nbsppanic(rcfpath) "start";
}

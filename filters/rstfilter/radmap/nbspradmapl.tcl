#!%TCLSH%
#
# $Id$
#
# Usage: nbspradmapl [-d <output_subdir>] [-l <latestname>] \
#			<site> <type> <nids_subdir>
#
# Example: nbspradmapl jua n0r digatmos/nexrad/nids
#
# This tool is designed for use by the web server to produce radar
# images on the fly. In the web server, the tool is executed from the
# Config(docRoot) so the <nids_subdir> must either be relative to that
# or the full directory path.
# 
package require cmdline;

set usage {nbspradmapl [-d <output_subdir>] [-l <latestname>]
    <site> <type> <nids_subdir>};
set optlist {{d.arg ""} {l.arg ""}};
array set option [::cmdline::getoptions argv $optlist $usage];

proc log_warn s {

    global argv0;

    set name [file tail $argv0];
    exec logger -t $name $s;
    puts stderr "$name: Error: $s";
}

proc log_err s {

    log_warn $s;
    exit 1;
}

#
# main
#

set f "/usr/local/etc/nbsp/filters.conf";
if {[file exists ${f}] == 0} {
   log_err "$f not found.";
}
source $f;

foreach f [list "rstfilter.init" "dafilter.init"] {
    set f [file join $common(libdir) $f];
    if {[file exists ${f}] == 0} {
	log_err "$f not found.";
    }
    source $f;
}
unset f;

if {$argc < 3} {
    log_err $usage;
}
set site [lindex $argv 0];
set type [lindex $argv 1];
set nidssubdir [lindex $argv 2];

if {$option(l) eq ""} {
    set option(l) $dafilter(rad_latestname);
}

set nidspath [file join $nidssubdir $site $type $option(l)];

set opts [list "-D" "awips=${type}${site}"];
if {$option(d) ne ""} {
    lappend opts "-d" $option(d) "-t" $option(d);
}

set status [catch {
    eval exec nbspradmap $opts $nidspath;
} errmsg];

if {$status != 0} {
    log_err $errmsg;
}

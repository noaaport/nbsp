#!%TCLSH%
#
# $Id$
#
package require cmdline;

set usage {nbspreconfigure [-d <destdir>] [-s <srcdir>] [<configname>]};
set optlist {{d.arg ""} {s.arg ""}};

# optional config file
set g(conffile) "/usr/local/etc/nbsp/nbspreconfigure.conf";
set g(configname) "";

# defaults
set g(srcdir) [file join [file dirname $g(conffile)] "dist"];
set g(destdir) [file join [file dirname $g(conffile)] "defaults" "configs"];
set g(fext) "-ex";

if {[file exists $g(conffile)]} {
    source $g(conffile)];
}

#
# functions
#
proc proc_remove_configs {} {

    global g;

    if {[file isdirectory $g(destdir)]} {
	set savedir [pwd];
	cd $g(destdir);
	foreach f [glob -nocomplain -directory . *] {
	    file delete $f;
	}
	cd $savedir;
    }
}

# main
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$argc != 0} {
    set g(configname) [lindex $argv 0];
}

if {$option(d) ne ""} {
    set g(destdir) $option(d);
}

if {$option(s) ne ""} {
    set g(srcdir) $option(s);
}

if {$g(configname) eq ""} {
    proc_remove_configs;
    return 0;
}

# Get the list of files
set flist [glob -nocomplain -directory $g(srcdir) *$g(configname)*$g(fext)];

if {[llength $flist] == 0} {
    puts "No files for $g(configname) found.";
    return 1;
}

# remove any existing config
proc_remove_configs;

# create the config directory if it does not exist
file mkdir $g(destdir);

foreach f $flist {
    file copy -force $f \
	[file join $g(destdir) [file tail [string trimright $f $g(fext)]]];
}

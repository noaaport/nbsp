#!%TCLSH%
#
# $Id$
#
package require cmdline;

#
set usage {nbspconfigure-dvbs2 [-r] [<configname>]};
set optlist {r};
#
# r => remove the files
#

# optional config file
set g(conffile) "/usr/local/etc/nbsp/nbspconfigure-dvbs2.conf";

# defaults
set g(srcdir) [file join [file dirname $g(conffile)] "dist"];
set g(udprecvsize_destpath) [file join [file dirname $g(conffile)]\
				"defaults" "nbspd.conf.d" "udprecvsize.conf"];
set g(sysctl_destpath) "/etc/%SYSCTLCONFLOCAL%";
#
set g(fext) "-ex";

if {[file exists $g(conffile)]} {
    source $g(conffile)];
}

#
# functions
#
proc proc_remove_files {} {

    global g;

    file delete $g(udprecvsize_destpath);
    file delete $g(sysctl_destpath);
}

# main
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$argc != 0} {
    puts "Argument ignored";
}

if {$option(r) == 1} {
    proc_remove_files;
    return 0;
}

# create the config directory if it does not exist
#
## file mkdir [file dirname $g(sysctl_destpath)];
file mkdir [file dirname $g(udprecvsize_destpath)];

foreach name [list sysctl udprecvsize] {
    set f [file join $g(srcdir) "${name}.conf$g(fext)"];
    file copy -force $f $g(${name}_destpath);
}

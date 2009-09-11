#!%TCLSH%
#
# Usage: nbspgribctlfiles <directory>
#
# where <directory> is relative to the gribfilter(datadir); for example
#
#	nbspgribctlfiles grb/<model>/<yyyymmddhh>
#
# This is a standalone program (not called by the filter) to manually
# create ctl files.

## The common grib tools initialization
set initfile "/usr/local/libexec/nbsp/nbspgribtools.init";
if {[file exists $initfile] == 0} {
    puts "$initfile not found.";
    return 1;
}
source $initfile;
unset initfile;

set usage {Usage: nbspctlfiles <directory>};

if {[llength $argv] != 1} {
    puts $usage;
    exit 1;
}
set directory [lindex $argv 0];

set save_dir [pwd];
cd $gribfilter(datadir);
set flist [glob -directory $directory -tails "*"];
cd $save_dir;

# /usr/local/libexec/nbsp (or wherever gribfilter-ctlfles is) must be in PATH,
# configures in filters.conf
set F [open "|gribfilter-ctlfiles" w];
fconfigure $F -translation binary -encoding binary;
foreach f $flist {
    set file [file join $directory $f];
    puts $F $file;
}
close $F;

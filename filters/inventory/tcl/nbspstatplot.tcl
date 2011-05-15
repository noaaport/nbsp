#!%TCLSH%
#
# $Id$
#
# Usage: nbspstatplot [-b basedir] [-d subdir]  [-f fmt] [-g fmtopts]
#	[-s [-m] [-h <hh>]] <inputfile>
#
# The tool will cd to the "basedir", create "subdir", and save the plot files
# with the default names
#
#	ftrans.<fmt>, fretrans.<fmt>, fmiss.<fmt>.
#       framesrcv.<fmt>, framesjumps.<fmt>, mbytes.<fmt>
#
# Without [-s] the <datafile> is the output from nbspstatplotdata.
# With [-s] the <inputfile> must an nbspd.status file, and then
# this tool calls nbspstatplodat to produce (and delete) the data file.
# If no inputfile is given with [-s], the default nbspd.status is used.
# With [-s], the options [-m] and [-h] are passed to nbspstatplotdata
# to produce minute data ([-m]) and give a backwards cutoff hour ([-h]).

package require cmdline;
package require fileutil;

set usage {nbspstatplot [-b basedir] [-d subdir] [-f fmt]
    [-s [-m] [-h <hh>]] <datafile>};
set optlist {{b.arg ""} {d.arg ""} {f.arg ""} {g.arg ""} s m {h.arg ""}};

## The common defaults
set _defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists ${_defaultsfile}] == 0} {
    puts stderr "${_defaultsfile} not found.";
    return 1;
}
source ${_defaultsfile};
unset _defaultsfile;

# The default configuration is shared between the filter and cmd-line tools
# and therefore it is in a separate file that is read by both.
set inv_init_file [file join $common(libdir) inventory.init];
if {[file exists $inv_init_file] == 0} {
    puts stderr "$inv_init_file not found.";
    return 1;
}
source $inv_init_file;
unset inv_init_file;

# Same with nbspd.init, which is needed for nbspd.status
set nbspd_init_file [file join $common(libdir) nbspd.init];
if {[file exists $nbspd_init_file] == 0} {
    puts stderr "$nbspd_init_file not found.";
    return 1;
}
source $nbspd_init_file;
unset nbspd_init_file;

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$argc > 1} {
    puts stderr $usage;
    exit 1;
} elseif {$argc == 1} {
    set inputfile [lindex $argv 0];
} else {
    set inputfile $nbspd(statusfile);
    set option(s) 1;
}

if {$option(s) == 0} {
    if {($option(m) != 0) || ($option(h) ne "")} {
	puts stderr $usage;
	exit 1;
    }
    # It is a data file. If the directory will be changed, get
    # the full path.
    if {($option(b) ne "") || ($option(d) ne "")} {
	set inputfile [file join [pwd] $inputfile];
    }
} else {
    # It is a nbspd.status file.
    set _opts [list];
    if {$option(m) != 0} {
	lappend _opts "-m";
    }
    if {$option(h) ne ""} {
	lappend _opts "-h" $option(h);
    }
    set databody [eval exec nbspstatplotdata ${_opts} $inputfile];
}

if {$option(b) ne ""} {
    cd $option(b);
}

if {$option(d) ne ""} {
    file mkdir $option(d);
    cd $option(d);
}

if {$option(f) ne ""} {
    set fmtoption "-f $option(f)";
} else {
    set fmtoption "";
}
if {$option(g) ne ""} {
    append fmtoption "-g $option(g)";
}

if {$option(s) == 1} {
    set status [catch {
	::fileutil::writeFile $inventory(plotfdatname) $databody;
	set inputfile $inventory(plotfdatname);
    } errmsg];
    if {$status != 0} {
	puts stderr $errmsg;
	exit 1;
    }
}

# It is possible that gnuplot emits some warnings (for example, when
# there is only one data point so that there is no range). To let gnuplot
# continue with the other plots we "catch" errors separately.

foreach {type name} {-t ftrans -r fretrans -m fmiss 
    -R framesrcv -J framesjumps -M mbytes} {
    set status [catch {
	eval exec nbspstatplot1 $type -o $name $fmtoption $inputfile;
    } errmsg];

    if {$status != 0} {
	puts stderr $errmsg;
    }
}

if {$option(s) == 1} {
    file delete $inputfile;
}

#!%TCLSH%
#
# $Id$
# 
# Usage: nbspradmosl [-b] [-c] [-C] [-d <outputdir>] [-g <globpatt>] [-i]
#                    [-o <outputname>] <dir>
#
# The argunment <dir> is a directory that contains the individual image files.
#
# Assumptions:
#   (1) The files are named such the they can be sorted by date
#   (2) The directory contains only the image files + the "latest" link file
#
# -b => background mode
# -c => create the <outputdir>
# -C => cd to <workdir> defined by nbspradmosl(Cdir)
# -d => output directory
# -g => glob pattern to choose input files
# -i => <dir> is relative to nbspradmos(Cdir)

set usage {nbspradmosl [-b] [-c] [-C] [-d outputdir] [-g globpatt] [i]
    [-o outputname] <dir>};
set optlist {b c C {d.arg ""} {g.arg ""} i {o.arg ""}};

proc log_warn s {

    ::nbsp::syslog::warn $s;
}

proc log_err s {

    ::nbsp::syslog::err $s;
}

proc nbspradmosl_makeloop {dir globpatt loopfpath} {

    global nbspradmosl;

    set flist [lsort [glob -nocomplain -directory $dir $globpatt]];
    set count [llength $flist];

    if {$count == 0} {
	return;
    }

    if {($nbspradmosl(count) > 0) && ($nbspradmosl(count) < $count)} {
      set flist [lrange $flist [expr $count - $nbspradmosl(count)] end];
    }

    set status [catch {
      eval exec $nbspradmosl(program) \
	$nbspradmosl(program_preoptions) $flist \
	$nbspradmosl(program_postoptions)  > $loopfpath;
    } errmsg];

    if {$status != 0} {
	file delete $loopfpath;
	log_err $errmsg;
    } 
}

## The common initialization
set initfile "/usr/local/libexec/nbsp/nbspradmos.init";
if {[file exists $initfile] == 0} {
   log_err "$initfile not found.";
}
source $initfile;

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$option(b) == 1} {
    ::nbsp::syslog::usesyslog
}

if {$argc == 0} {
    log_err $usage;
}

set dir [lindex $argv 0];

if {$option(C) == 1} {
    cd $nbspradmos(Cdir);
}

if {$option(i) == 1} {
    set dir [file join $nbspradmos(Cdir) $dir];
}

if {$option(o) ne ""} {
    set loopfpath $option(o);
} else {
    set loopfpath $nbspradmosl(loopfilename)
}

if {$option(d) ne ""} {
    if {$option(c) == 1} {
	file mkdir $option(d);
    }
    set loopfpath [file join $option(d) $loopfpath];
}

if {$option(g) ne ""} {
    set globpatt $option(g);
} else {
    set globpatt $nbspradmosl(globpatt);
}

nbspradmosl_makeloop $dir $globpatt $loopfpath;

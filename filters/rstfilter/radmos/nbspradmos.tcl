#!%TCLSH%
#
# $Id$
# 
# Usage: nbspradmos [-b] [-d <outputdir>] [-f <fmt>] [-o <outputname>]
#	 [-k] [-l <logfile>]
#        [-s <devsize>] [-t <tmpdir>] [-D <defs>]
#        [-r <rcfile> | -R <rcfilepath>] <inputfile>
#
# -D => key=value,... comma separated list of gpmap(key)=var pairs
# -b => background mode
# -d => output directory
# -f => output file format
# -o => output file name
# -k => keep the log file (the default is to delete it)
# -l => use the given logfile (implies -k).
# -s => device size
# -t => cd to tmp directory (all partial paths are still relative
#       to the current directory.
# -r => rcfile, searched in the standard directories
# -R => rcfilepath, used as given
#
# This program ends up calling gdplot2.
# If the <rcfile> is not specified, the program uses the same logic as
# nbspradmap to search for the default file "radcomp.rc".
#
package require cmdline;

set usage {nbspradmos [-b] [-d outputdir] [-f fmt] [-o outputname]
    [-k] [-l <logfile>] [-s <devsize>] [-t <tmpdir>]
    [-D <defs>] [-r <rcfile> | -R <rcfilepath>]};

set optlist {b {d.arg ""} {f.arg ""} {o.arg ""} k {l.arg ""}
    {s.arg ""} {t.arg ""} {D.arg ""} {r.arg ""} {R.arg ""}};
array set option [::cmdline::getoptions argv $optlist $usage];

proc log_warn s {

    global argv0;
    global option;

    set name [file tail $argv0];
    if {$option(b) == 0} {
        puts "$name: $s";
    } else {
        exec logger -t $name $s;
    }
}

proc log_err s {

    log_warn $s;
    exit 1;
}

proc source_template {rcfile} {
#
# The template is sourced in a function so that the template cannot affect
# the main script environment.
#
    global gdplot2 gpcolor;
    
    source $rcfile;
}

## The common defaults (gempak.env is needed and the filter library are neded)
set defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists $defaultsfile] == 0} {
   log_err "$defaultsfile not found.";
}
source $defaultsfile;

if {[file exists $gempak(envfile)] == 0} {
    log_err "$gempak(envfile) not found.";
}
source $gempak(envfile);

# Defaults
set nbspradmos(localconfdirs) $common(localconfdirs);
set nbspradmos(conf)      [file join $common(confdir) "nbspradmos.conf"];
#
set nbspradmos(rcsubdir)  [file join "gdplot2" "rad"];
set nbspradmos(rcfile)    "radmos.n0r.rc";
#
set gdplot2(devfmt) "gif";
set gdplot2(devsize) "";

# optional config file
if {[file exists $nbspradmos(conf)]} {
    source $nbspradmos(conf);
}

#
# main
#
set argc [llength $argv];
if {$argc == 0} {
    log_err $usage;
}
set gdplot2(gdfile) [lindex $argv 0];

if {$option(R) eq ""} {
    # Search for the rcfile 
    source $common(filterslib);
    if {$option(r) eq ""} {
	set option(r) $nbspradmos(rcfile);
    }
    set option(rcfile) [filterlib_find_conf $option(r) \
			    $nbspradmos(localconfdirs) \
			    $nbspradmos(rcsubdir)];
} else {
    set option(rcfile) $option(R);
}

# Check that it exists
if {[file exists $option(rcfile)] == 0} {
    log_err "$option(rcfile) not found.";
}

# Definitions - Override any gdplot2() settings in conf file.
if {$option(D) ne ""} {
    set Dlist [split $option(D) ","];
    foreach pair $Dlist {
        set p [split $pair "="];
        set var [lindex $p 0];
        set val [lindex $p 1];
        set gdplot2($var) $val;
    }
}

if {$option(f) ne ""} {
    set gdplot2(devfmt) $option(f);
}

if {$option(s) ne ""} {
    set gdplot2(devsize) $option(s);
}

if {$option(o) ne ""} {
    set gdplot2(devfile) $option(o);
    set outrootname [file rootname [file tail $gdplot2(devfile)]];
} else {
    set outrootname [file rootname [file tail $gdplot2(gdfile)]];
    set gdplot2(devfile) $outrootname;
    append gdplot2(devfile) "." $gdplot2(devfmt);
}

if {$option(l) eq ""} {
    append logfile $outrootname ".log";
} else {
    set logfile $option(l);
}

if {$option(d) ne ""} {
    set gdplot2(devfile) [file join $option(d) $gdplot2(devfile)];
}

if {$option(t) ne ""} {
    if {[file isdirectory $option(t)] == 0} {
	log_err "No such directory: $option(t)";
    } else {
	set cwd [pwd];
	set option(rcfile) [file join $cwd $option(rcfile)];
	set gdplot2(gdfile) [file join $cwd $gdplot2(gdfile)];
	set gdplot2(devfile) [file join $cwd $gdplot2(devfile)];
	# If a logfile was given, it is relative to the original dir.
	if {$option(l) ne ""} {
	    set logfile [file join $cwd $logfile];
	}
	cd $option(t);
    }
}

file delete "gemglb.nts" "last.nts";
if {$option(k) == 0} {
    file delete $logfile;
}

set status [catch {
    source_template $option(rcfile);
    if {([info exists gdplot2(script)] == 0) || \
	    ([info exists gpcolor(script)] == 0)} {
	return;
    }
    set fout [open "|gpcolor >& $logfile" w];
    fconfigure $fout -translation binary -encoding binary;
    set script [subst $gpcolor(script)];
    puts $fout $script;
    close $fout;
    unset fout;

    set fout [open "|gdplot2 >& $logfile" w];
    fconfigure $fout -translation binary -encoding binary;
    set script [subst $gdplot2(script)];
    puts $fout $script;
} errmsg];

if {[info exists fout]} {
    catch {close $fout};
}
catch {exec gpend};

file delete "gemglb.nts" "last.nts";
if {$option(k) == 0} {
    file delete $logfile;
}

if {$status != 0} {
    # In case gdplot2 created the file.
    file delete $gdplot2(devfile);
    log_err $errmsg;
}

if {[info exists gdplot2(post)]} {
    eval $gdplot2(post);
}

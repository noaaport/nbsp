#!%TCLSH%
#
# $Id$
#
# Usage: nbspstatplot1 [-o <plotname>] [-f fmt] [-g fmtopts]
#	[-t|-r|-m|-R|-J|-M] <datafile>
#
# [-t] => files transmitted
# [-r] => files retransmitted
# [-m] => files missed
# [-R] => frames received
# [-J] => frames jumps
# [-M] => (M)bytes received
#
# The <datafile> is the output from nbspstatplotdata. It can be an hourly
# or minutely data file. The templates know how to to deal with both.
# Without options, [-t] is the default. The tool will 
# save the file in <plotname>.<fmt> or with the name
# $inventory(plotfname).<fmt>.

package require cmdline;

set usage {nbspstatplot1 [-o outputfile [-b basedir] [-d subdir]]
    [-f fmt] [-g fmtoptions] [-t|-r|-m|-R|-J|-M] <datafile>};
set optlist {{o.arg ""} {b.arg ""} {d.arg ""} {f.arg ""} {g.arg ""}
    t r m R J M};

array set option [::cmdline::getoptions argv $optlist $usage];

proc check_option_conflict optionarray {

    upvar $optionarray option; 

    set trmRJM_conflict 0;

    foreach key [list t r m R J M] {
	if {$option($key) == 1} {
	    incr trmRJM_conflict;
	}
    }
    
    if {$trmRJM_conflict > 1} {
	return 1
    }

    return 0;
}

proc choose_plot_template {optionarray} {

    upvar $optionarray option;
    global inventory;

    set _template(t) $inventory(plotftransrc);
    set _template(r) $inventory(plotfretransrc);
    set _template(m) $inventory(plotfmissrc);
    set _template(R) $inventory(plotframesrcvrc);
    set _template(J) $inventory(plotframesjumpsrc);
    set _template(M) $inventory(plotbytesrc);

    set templatename "";
    # First get the name of the template based on the option argument.
    foreach key [list t r m R J M] {
	if {$option($key) == 1} {
	    set templatename $_template($key);
	    break;
	}
    }

    if {$templatename eq ""} {
	set templatename $inventory(plotftransrc);
    }

    # Look for it in the template directories and use the last one found.
    set _tmpl "";
    set _sub $inventory(plottemplatesubdir);
    foreach d $inventory(plottemplatedirs) {
	if {[file exists [file join $d ${_sub} $templatename]]} {
	    set _tmpl [file join $d ${_sub} $templatename];
	}
    }

    if {${_tmpl} eq ""} {
	puts "$templatename not found.";
	exit 1;
    }

    return ${_tmpl};
}

## The common defaults
set _defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists ${_defaultsfile}] == 0} {
    puts "${_defaultsfile} not found.";
    return 1;
}
source ${_defaultsfile};
unset _defaultsfile;

# The default configuration is shared between the filter and cmd-line tools
# and therefore it is in a separate file that is read by both.
set inv_init_file [file join $common(libdir) inventory.init];
if {[file exists $inv_init_file] == 0} {
    puts "$inv_init_file not found.";
    return 1;
}
source $inv_init_file;
unset inv_init_file;

#
# main
#
set argc [llength $argv];
if {$argc != 1} {
    puts $usage;
    exit 1;
} else {
    set gplot(datafile) [lindex $argv 0];
}

# Check for conflicting options
if {[check_option_conflict option] != 0} {
    puts $usage;
    exit 1;
} else {
    set gnuplot(template) [choose_plot_template option];
}

set gplot(fmt) $inventory(plotfmt);
set gplot(fmtoptions) $inventory(plotfmtoptions);
if {($option(f) ne "") && ($option(f) ne $inventory(plotfmt))} {
    set gplot(fmt) $option(f);
    set gplot(fmtoptions) "";
}
if {$option(g) ne ""} {
    set gplot(fmtoptions) $option(g);
}

if {$option(o) ne ""} {
    set gplot(output) $option(o).$gplot(fmt);
} else {
    set gplot(output) $inventory(plotfname).$gplot(fmt);
}

# Initialize the gnuplot(script) and gnuplot(post) variables
set gnuplot(script) "";
set gnuplot(post) "";
source $gnuplot(template);

set status [catch {
    set F [open "|gnuplot > /dev/null" w];
    puts $F [subst -nobackslashes $gnuplot(script)];
} errmsg];

if {$status != 0} {
    puts $errmsg;
}

if {[info exists F]} {
    set status [catch {close $F} errmsg];
}

# gnuplot sometimes throws a warning. We try to catch it and _not_
# flag it as an error.
if {($status != 0) && ([regexp -nocase {warning} $errmsg] == 0)} {
    puts $errmsg;
    exit 1;
}

if {$gnuplot(post) ne ""} {
    eval $gnuplot(post);
}

exit 0;

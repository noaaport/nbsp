#!%TCLSH%
#
# $Id$
#
# Usage: nbspstatplot1 [-o <plotname>] [-f fmt] [-g fmtopts]
#	[-M|-t|-r|-m] <datafile>
#
# [-M] => plot (M)bytes received
# [-t] => files transmitted
# [-r] => files retransmitted
# [-m] => files missed
#
# The <datafile> is the output from nbspstatplotdat. It can be an hourly
# or minutely data file. The templates know how to to deal with both.
# Without options, [-t] is the default. The tool will 
# save the data in <plotname>.<fmt> or with the name
# $inventory(plotfname).<fmt>.

package require cmdline;

set usage {nbspstatplot1 [-o outputfile [-b basedir] [-d subdir]]
    [-f fmt] [-g fmtoptions] [-M|-t|-r|-m] [-n] <datafile>};
set optlist {{o.arg ""} {b.arg ""} {d.arg ""} {f.arg ""} {g.arg ""} M t r m n};

array set option [::cmdline::getoptions argv $optlist $usage];

proc check_option_conflict optionarray {

    upvar $optionarray option; 

    set Mtrm_conflict 0;

    if {$option(M) == 1} {
	incr Mtrm_conflict;
    }
    if {$option(t) == 1} {
	incr Mtrm_conflict;
    }
    if {$option(r) == 1} {
	incr Mtrm_conflict;
    }

    if {$option(m) == 1} {
	incr Mtrm_conflict;
    }
    
    if {$Mtrm_conflict > 1} {
	return 1
    }

    return 0;
}

proc choose_plot_template {optionarray} {

    upvar $optionarray option;
    global inventory;

    set templatename "";
    # First get the name of the template based on the option argument.
    if {$option(M) == 1} {
	set templatename $inventory(plotbytesrc);
    } elseif {$option(t) == 1} {
	set templatename $inventory(plotftransrc);
    } elseif {$option(r) == 1} {
	set templatename $inventory(plotfretransrc);
    } elseif {$option(m) == 1} {
	set templatename $inventory(plotfmissrc);
    }

    if {$templatename == ""} {
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

    if {${_tmpl} == ""} {
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

if {$option(o) != ""} {
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

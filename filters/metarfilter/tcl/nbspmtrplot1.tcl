#!%TCLSH%
#
# $Id$
#
# Usage: nbspmtrplot1 [-p <param> [-g fmtoptions]
#	 [-i datafile | -d outputdatafile] [-m] [-o <plotfile>] <station>
#
# If [-o] is given it writes the plot to <plotfile>.<fmt>, otherwise
# <station>.<fmt> is used. The [-p] option determines the variable
# to plot. <param> is one of pre, temp (TDH), wspeed or wdir (or any
# value for which $metarfilter(plotrc_<param>) gives a template name.
# If none is given, temp is assumed.
# The default is to keep the data file that is generated, unless [-k]
# is specified. The [-i] option can specify a file to read the data from. The
# file can be (and must be in the same format as) the datafile that 
# nbspmtrplotdat generates. The name of the output data file can be specified
# with [-d]. Any intermediate directories
# appearing in the [-d] or [-o] options are not created unless [-m] is given.
# The [-f fmt] option can specify a file format for gnuplot.
# The default is png (as defined and editable in metarfilter.conf)
# but other possibilities are gif, jpeg and others. The [-g] option
# can specify additional options as they appear in the ``set terminal''
# gnuplot command.

package require cmdline;
set usage {nbspmtrplot1 [-p <param>] [-k] [-f fmt] [-g fmtoptions]
    [-i inputdatafile | -d outputdatafile] [-m] [-o <plotfile>] <station>};
set optlist {k m {f.arg ""} {g.arg ""} {i.arg ""} {d.arg ""} {o.arg ""}
    {p.arg "temp"}};

proc check_option_conflict optionarray {

    upvar $optionarray option; 

    set di_conflict 0;

    if {$option(d) != ""} {
	incr di_conflict;
    }
 
    if {$option(i) != ""} {
	incr di_conflict;
    }
    if {$di_conflict > 1} {
	return 1
    }

    return 0;
}

proc choose_plot_template {optionarray station} {

    upvar $optionarray option;
    global metarfilter;

    # First get the name of the template based on the -p argument.
    set param $option(p);
    if {[info exists metarfilter(plotrc_$param)] == 0} {
	puts "Invalid parameter name: $param.";
	exit 1;
    }

    set templatename $metarfilter(plotrc_$param);

    # Look for it in the template directories and use the last one found.
    # First in the default directories, then in the individual station
    # directories.
    set _tmpl "";
    set subdir $metarfilter(plottemplatesubdir);
    foreach d $metarfilter(plottemplatedirs) {
	if {[file exists [file join $d $subdir $templatename]]} {
	    set _tmpl [file join $d $subdir $templatename];
	}
    }
    set subdir [file join $metarfilter(plottemplatesubdir) $station];
    foreach d $metarfilter(plottemplatedirs) {
	if {[file exists [file join $d $subdir $templatename]]} {
	    set _tmpl [file join $d $subdir $templatename];
	}
    }
    if {${_tmpl} == ""} {
	puts "$templatename not found.";
	exit 1;
    }

    return ${_tmpl};
}

proc compute_plot_xtics {datalist} {

    set n 0;
    set i 0;
    set xtics "";

    foreach line $datalist {
	incr i;
	incr n;
	if {$i == 1} {
	    set fields [split $line];
	    set day [lindex $fields 1];
	    set time [lindex $fields 2];
	    if {$xtics eq ""} {
		append xtics "(" "\"$day\\n$time\"" " " $n;
	    } else {
		append xtics "," "\"$day\\n$time\"" " " $n;
	    }
	}

	if {$i == 6} {
	    set i 0;
	}
    }
    append xtics ")";

    return $xtics;
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
set metar_init_file [file join $common(libdir) metarfilter.init];
if {[file exists $metar_init_file] == 0} {
    puts "metarfilter disabled: $metar_init_file not found.";
    return 1;
}
source $metar_init_file;
unset metar_init_file;

array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];
if {$argc != 1} {
    puts $usage;
    exit 1;
} else {
    set station [lindex $argv 0];
}

# Check for conflicting options
if {[check_option_conflict option] != 0} {
    puts $usage;
    exit 1;
} else {
    set gnuplot(template) [choose_plot_template option $station];
}

if {$option(i) != ""} {
    set gplot(datafile) $option(i);
} else {
    if {$option(d) != ""} {
	set gplot(datafile) $option(d);
	if {$option(m) == 1} {
	    file mkdir [file dirname $option(d)];
	}
    } else {
	set gplot(datafile) ${station}$metarfilter(plotdataext);
    }

    set status [catch {
	exec nbspmtrplotdat -o $gplot(datafile) $station;
    } errmsg];
    if {$status != 0} {
	puts $errmsg;
	exit 1;
    }
}
set data [string trimright [exec cat $gplot(datafile)]];
set datalist [split $data "\n"];

set gplot(fmt) $metarfilter(plotfmt);
set gplot(fmtoptions) $metarfilter(plotfmtoptions);
if {($option(f) ne "") && ($option(f) ne $metarfilter(plotfmt))} {
    set gplot(fmt) $option(f);
    set gplot(fmtoptions) "";
}
if {$option(g) ne ""} {
    set gplot(fmtoptions) $option(g);
}
set gplot(start) [string range [lindex $datalist 0] 5 12];
set gplot(end) [string range [lindex $datalist end] 5 12];
set gplot(station) $station;
set gplot(STATION) [string toupper $station];
set gplot(output) $gplot(station).$gplot(fmt);
if {$option(o) != ""} {
    set gplot(output) $option(o).$gplot(fmt);
    if {$option(m) == 1} {
	file mkdir [file dirname $option(o)];
    }
}

# Compute the gnuplot xtics
set gplot(xtics) [compute_plot_xtics $datalist];

# Initialize the gnuplot(script) and gnuplot(post) variables
set gnuplot(script) "";
set gnuplot(post) "";
source $gnuplot(template);

set status [catch {
    set F [open "|gnuplot > /dev/null" w];
    fconfigure $F -translation binary -encoding binary;
    puts $F [subst -nobackslashes -nocommands $gnuplot(script)];
} errmsg];

if {$status != 0} {
    puts $errmsg;
}

if {[info exists F]} {
    set status [catch {close $F} errmsg];
}

if {$option(k) != 0} {
    file delete $gplot(datafile);
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

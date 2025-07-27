#!%TCLSH%
#
# Usage: nbspcraftinsert [-b] [-v] [-m] [-n] [-r]
#		[-d <nbsp_spool_dir>] [-f <nbsp_infifo>]
#		[-R <nonop_flist>] [-w <level2_wmoid>] <inputfile>
#
# This is cmdline tool with no configuration file.
#
# This script takes as argument the same kind of file as "craftinsert",
# and processes the file in the same way, but with these two differences
#
# (i) takes the configurable options from the cmd line instead
#     of a configuration file as is the case with craftinsert.
# (ii) does not use "umask" and therefore does not require tclx.
#
# This program is designed to be used from within nbsp itself, or a program
# (like npemwin) that runs as the noaaport user (whence there is no need to
# call "umask"), while craftinsert is meant to be used by other programs,
# like ldm (and therefore the need to use "umask").
#
# -b => background mode
# -v => verbose
# -m => move the file to the nbsp spool (default is to let nbspinsert do it)
# -n => delete (don't keep) the input file after insert (default is to keep it)
# -r => delete also non-operational files
# -R => list of non_operational files ("|"-separated)
# -d => nbsp spool directory
# -f => path to the nbsp input fifo file (infifo)
# -w => the wmoid of the level2 files

#
# libraries
#
package require cmdline;

#
# functions
#
proc msg {s} {

    global g;

    if {$g(verbose) == 0} {
	return;
    }

    if {$g(background) == 0} {
	puts stdout "$g(name): $s";
    } else {
	exec logger -t $g(name) $s;
    }
}

proc err {s} {
    
    global g;

    if {$g(background) == 0} {
	puts stderr "$g(name): $s";
    } else {
	exec logger -t $g(name) $s;
    }
}

proc proc_nbsp {input_path} {
    #
    # This function mimicks the same function in craftinsert, without
    # the use of the "umask" proc.
    #
    # This script executes "nbspinsert", which was originally a tcl script.
    # The script was invoked as
    #
    # exec nbspinsert $finfo
    #
    # and therefore finfo was passed as a string and nbspinsert was
    # responsible for splitting the string into the various elements.
    # In jan2023 the nbspinsert program was replaced by a C program
    # (to ease the implementation of locking the fifo) and that new
    # nbspinsert expects the elements of finfo to be passed as
    # separate arguments, not as one string; i.e.
    #
    # nbspinsert seq type cat code npchidx fname fpath
    #
    # Whence the invocation of this version of nbspinsert must be done
    # now with eval if we insist on using a finfo string, or call it
    # with separate arguments
    #
    # eval exec nbspinsert finfo
    # exec nbspinsert seq type cat code npchidx fname fpath
    #
    # We use the latter.
    #
    global g;
	     
    set wmoid $g(nbsp_wmoid);

    # Extract info from input name
    set input_name [file tail $input_path];

    # Ignore non-operational products
    if {[regexp $g(non_operational) $input_name]} {
	msg "Ignoring non-operational file: $input_name";
	if {$g(deletenonop) == 1} {
	    file delete $input_path;
	}
	return;
    }

    if {[regexp {^([A-Z]{4})_(\d{8})_(\d{4})} \
	     $input_name match STATION ymd hm] == 0} {
	return -code error "Invalid file name: $input_name";
    }

    set station [string tolower $STATION];
    append dhm [string range $ymd 6 7] $hm;

    # For the sequence number use the milliseconds seconds since midnight
    append _datestr $ymd "T000000";
    set s1 [clock scan ${_datestr} -gmt 1];
    append s1 "000";
    set s2 [clock milliseconds];
    set seq [expr $s2 - $s1];
    
    append fname $station "_" $wmoid;
    append fbasename $fname "." $dhm "_" $seq;
    set fpath [file join $spooldir $station $fbasename];

    ##  finfo no longer used
    # set finfo "$seq 0 0 0 0 $fname $fpath";
    set type 0;
    set cat 0;
    set code 0;
    set npchidx 0;

    # The spooldir must exist - we create (if necessary) the parent subdir
    if {[file isdirectory $spooldir] == 0} {
       return -code error "Spool directory does not exist: $spooldir";
    }
    file mkdir [file dirname $fpath];

    if {$g(mvtospool) == 0} {
	set status [catch {
	    # exec nbspinsert -i -f $g(nbsp_infifo) \
	    #		       $finfo < $input_path;
	    exec nbspinsert -i -f $g(nbsp_infifo) \
		$seq $type $cat $code $npchidx $fname $fpath < $input_path;
	    if {$g(delete) == 1} {
		file delete $input_path;
	    }
	} errmsg];
    } else {
	file rename -force $input_path $fpath;
	set status [catch {
	    # exec nbspinsert -f $g(nbsp_infifo) $finfo;
	    exec nbspinsert -f $g(nbsp_infifo) \
		$seq $type $cat $code $npchidx $fname $fpath;
	} errmsg];

	if {$status != 0} {
	    file delete $fpath;
	}
    }

    if {$status != 0} {
	return -code error $errmsg;
    }
}

proc print_conf {} {

    global g;
    
    puts $g(background);
    puts $g(verbose);
    puts $g(nbsp_spooldir);
    puts $g(nbsp_infifo);
    puts $g(nbsp_wmoid);
    puts $g(mvtospool);
    puts $g(delete);
    puts $g(deletenonop);
    puts $g(non_operational);

    exit 0;
}

# options
set g(background) 0;
set g(verbose) 0;

#
# configuration
#

# defaults
set g(nbsp_spooldir) "/var/noaaport/nbsp/spool";
set g(nbsp_infifo) "/var/run/nbsp/infeed.fifo";
set g(nbsp_wmoid) "level2";
set g(mvtospool) 0;  # move to spool or insert (default is insert)
set g(delete) 0;     # delete after insert
set g(deletenonop) 0; # delete also non-operational files
set g(non_operational) {DAN1|DOP1|FOP1|NOP4|ROP4|NOP3};

# options
set g(background) 0;
set g(verbose) 0;

# overrides
set usage {nbspcraftinsert [-b] [-v] [-m] [-n] [-r]
    [-d <nbsp_spool_dir>] [-f <nbsp_infifo>]
    [-R <nonop_flist>] [-w <level2_wmoid>] <inputfile>
};
set optlist {b v m n r {d.arg ""} {f.arg ""} {R.arg ""} {w.arg ""}};

# variables
set g(name) [file tail $argv0];
set status 0;

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];

# Get the input path
set argc [llength $argv];
if {$argc == 0} {
    err "Requires an argument.";
}
set input_path [lindex $argv 0];

# reset configurable options
set g(background) $option(b);
set g(verbose) $option(v);
set g(mvtospool) $option(m);
set g(delete) $option(n);	# delete after insert
set g(deletenonop) $option(r); # delete also non-operational files
#
if {$option(d) ne ""} {
    set g(nbsp_spooldir) $option(d);
}
if {$option(f) ne ""} {
    set g(nbsp_infifo) $option(f);
}
if {$option(R) ne ""} {
    set g(non_operational) $option(R);
}
if {$option(w) ne ""} {
    set g(nbsp_wmoid) $option(w);
}

# test
## print_conf;

set status [catch {
    proc_nbsp $input_path;
} errmsg];

if {$status != 0} {
    err $errmsg;
}

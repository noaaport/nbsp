#!%TCLSH%
#
# $Id$
#
# For installation see "craftinsert.README"
#
package require Tclx;	# umask

# defaults
set craftinsert(confdir) "/usr/local/etc/nbsp";
set craftinsert(conf) [file join $craftinsert(confdir) "craftinsert.conf"];
set craftinsert(localconfdirs) [list \
	 [file join $craftinsert(confdir) "defaults"] \
	 [file join $craftinsert(confdir) "site"]];
#
# These are configurable in the configuration file(s)
#
set craftinsert(nbspd_enable) 0;
set craftinsert(nbspd_spooldir) "/var/noaaport/nbsp/spool";
set craftinsert(nbspd_infifo) "/var/run/nbsp/infeed.fifo";
set craftinsert(nbspd_wmoid) "level2";
set craftinsert(mvtospool) 0;  # move to spool or insert (default is insert)
set craftinsert(delete) 0;     # delete after insert
set craftinsert(umask) "002";
#
set craftinsert(ldm_fext) ".tmp";    # must match what is used in pqact.conf
set craftinsert(non_operational) {DAN1|DOP1|FOP1|NOP4|ROP4|NOP3};

proc msg {s} {

    global argv0;

    set name [file tail $argv0];

    puts stdout "$name: $s";
    exec logger -t $name $s;
}
 
proc err {s} {

    global argv0;

    set name [file tail $argv0];

    puts stderr "$name: $s";
    exec logger -t $name $s;

    exit 1;
}

proc proc_nbsp {ppath} {

    global craftinsert;

    set spooldir $craftinsert(nbspd_spooldir);
    set wmoid $craftinsert(nbspd_wmoid);

    # Extract info from input name
    set input_name [file tail $ppath];

    # Ignore non-operational products
    if {[regexp $craftinsert(non_operational) $input_name]} {
	msg "Ignoring non-operational file: $input_name";
	return;
    }

    if {[regexp {^([A-Z]{4})_(\d{8})_(\d{4})} \
	     $input_name match STATION ymd hm] == 0} {
	return -code error "Invalid file name: $input_name";
    }

    set station [string tolower $STATION];
    append dhm [string range $ymd 6 7] $hm;

    # For the sequence number use the unix seconds
    append _datestr $ymd "T" $hm "00";
    set seq [clock scan ${_datestr} -gmt 1];
    
    append fname $station "_" $wmoid;
    append fbasename $fname "." $dhm "_" $seq;
    
    set fpath [file join $spooldir $station $fbasename];
    set finfo "$seq 0 0 0 0 $fname $fpath";

    set oldmask [umask];
    umask $craftinsert(umask);

    if {$craftinsert(mvtospool) == 0} {
	exec nbspinsert -i -f $craftinsert(nbspd_infifo) $finfo < $ppath;
	if {$craftinsert(delete) == 1} {
	    file delete $ppath;
	}
    } else {
        # The spooldir must exist
        if {[file isdirectory $spooldir] == 0} {
	    return -code error "Spool directory does not exist: $spooldir";
        }
        file mkdir [file dirname $fpath];

	file rename -force $ppath $fpath;
	set status [catch {
	    exec nbspinsert -f $craftinsert(nbspd_infifo) $finfo;
	} errmsg];

	if {$status != 0} {
	    file delete $fpath;
	    return -code error $errmsg;
	}
    }

    umask $oldmask;
}

#
# main
#
# Read optional config file
if {[file exists $craftinsert(conf)]} {
    source $craftinsert(conf);
}

set argc [llength $argv];
if {$argc == 0} {
    err "Requires an argument.";
}

# Get the (partial) path
set ppath [lindex $argv 0];

# Strategy -
#
# Ldm is setup such that:
#
# (1) the ppath argument is of the form
#    <directory>/<name>
#
# (2) the tmp file that contains the data is
#    ${ppath}.tmp
#

append tmppath $ppath $craftinsert(ldm_fext);
if {[file exists $tmppath]} {
    file rename -force $tmppath $ppath;
}

if {$craftinsert(nbspd_enable) == 1} {
    set status [catch {
	proc_nbsp $ppath;
    } errmsg];

    if {$status != 0} {
	err $errmsg;
    }
}

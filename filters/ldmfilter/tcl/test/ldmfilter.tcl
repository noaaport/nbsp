#!/usr/bin/tclsh
#
# $Id$
#

## The common defaults
set filters_init_file "/usr/local/libexec/nbsp/filters.init";
if {[file exists $filters_init_file] == 0} {
	puts "ldmfilter disabled: $filters_init_file not found.";
	return 1;
}
source $filters_init_file;
unset filters_init_file;

if {$filters_init_status == 1} {
	return 1;
}
unset filters_init_status;

# Nbsp packages used by this filter
package require nbsp::ldm;

# default configuration file and rc script
set ldmfilter(conf)	"$common(confdir)/ldmfilter.conf";
set ldmfilter(rc)	"$common(confdir)/ldmfilter.rc";
set ldmfilter(prerc)	"$common(confdir)/ldmfilterpre.rc";
set ldmfilter(postrc)	"$common(confdir)/ldmfilterpost.rc";
set ldmfilter(rcdir)	"$common(rcdir)/ldm";
set ldmfilter(localconfdirs) $common(localconfdirs);
# The file for determining the name of the sat files
set ldmfilter(satdef)     [file join $common(confdir) "gpfilter-sat.def"];
set ldmfilter(useseqnumid) 1;
#
set ldmfilter(verbose) 1;
#
set ldmfilter(ldmpq)	"/var/ldm/ldm.pq";
#
set ldmfilter(feeds,WMO) 15;
set ldmfilter(feeds,HDS) 4;
set ldmfilter(feeds,NEXRAD) 134217728;
set ldmfilter(feeds,NIMAGE) 2097152;
set ldmfilter(feeds,NGRID) 8388608;
#
set ldmfilter(NIMAGE,unzflag) 0;
#
# Variables
#
set ldmfilter(rcfiles)		[list];
set ldmfilter(ccbsaved)		$common(ccbsaved);
set ldmfilter(ccbsize)		$common(ccbsize);
#
set ldmfilter(condition)	[list];
set ldmfilter(action)		[list];
#
set ldmfilter(satdef_sourced)    0;

#
# Read the default configuration file.
#
if {[file exists $ldmfilter(conf)] == 1} {
    source $ldmfilter(conf);
}

#
# Read once and for all the sat definitions file and copy the
# gpfilter(satdef,...) variables to ldmfilter(satdef,...) for easier
# reference in the mk_ldm_sat_prodid function.
#
if {[file exists $ldmfilter(satdef)]} {
    source $ldmfilter(satdef);
    set ldmfilter(satdef_sourced) 1;

    foreach k [array names gpfilter "satdef,*"] {
	set ldmfilter($k) $gpfilter($k);
    }
}

# The main rc file is required
if {[file exists $ldmfilter(rc)] == 0} {
    log_msg "Filter disabled: $ldmfilter(rc) not found.";
    return 1;
}

# Build the list of rc files
set ldmfilter(rcfiles) [filterlib_get_rcfiles [file tail $ldmfilter(rc)] \
        $ldmfilter(localconfdirs) $ldmfilter(rcdir) \
	$ldmfilter(prerc) $ldmfilter(postrc)];

# Load the rules sets
source $ldmfilter(rc);

proc main {argc argv} {

    global errorInfo;

    # gets returns -1 on eof. In addition the server explicitly
    # signals the filters to quit by sending a blankline
    # (gets returns 0 in this case, and finfo is "").

    while {[gets stdin finfo] >= 0} {
	if {$finfo == ""} {
	    log_msg "Received request to quit.";
	    break;
	}

	set status [catch {process $finfo} errmsg];
	if {$status == 1} {
	    log_msg "Error processing $finfo";
	    log_msg $errmsg;
	    log_msg $errorInfo;
	}
    }
}

proc process {finfo} {

    global ldmfilter;

    filterlib_get_rcvars rc $finfo -dogrib;

    # Initialize to no match.
    # If rc_status is returned as zero, then it matched a rule.
    # The variables 
    #   rc_ldmfeed
    #   rc_prodid
    # contain the feedtype and prodid that should be used to insert
    # the product in the ldm queue.

    set rc_status 1;
    set rc_ldmfeed "";
    set rc_prodid "";

    # Evaluate the condition/action pairs, from all sets.
    set i 0;		# counts the sets
    foreach condlist $ldmfilter(condition) {
        set actionlist [lindex $ldmfilter(action) $i];
	set j 0;	# counts the rules with each set
	foreach c $condlist {
	    set a [lindex $actionlist $j];
            if {[expr $c]} {
                eval $a;
            }
	    incr j;
        }
        incr i;
    }

    if {$rc_status == 0} {
	filter_ldmfeed $rc(seq) $rc(fpath) $rc_ldmfeed $rc_ldmprodid;
    }
}

proc filter_ldmfeed {seq fpath feedtype prodid} {
#
# Here, ``feedtype'' is the name, such as NIMAGE.
#
    global ldmfilter;

    set opts [list];

    # The satellite name can be empty in case of error from the
    # mk_ldm_sat_prodid function.
    if {$prodid eq ""} {
	log_msg "Error sending $fpath: Empty prodid.";
	return;
    }

    # This should not happen.
    if {[info exists ldmfilter(feeds,$feedtype)] == 0} {
	log_msg "Error sending $fpath: Undefined feed $feedtype.";
	return;
    }
    set feed $ldmfilter(feeds,$feedtype);

    # For satellite file, do not add header/trailer, and there is no ccb.
    if {$feedtype eq "NIMAGE"} {
	lappend opts "-n";
    } else {
	lappend opts "-g";
	if {$ldmfilter(ccbsaved) == 1} {
	    lappend opts "-c" $ldmfilter(ccbsize);
	}
    }
 
    if {$ldmfilter(useseqnumid) == 1} {
    	lappend opts "-m";
    }

    if {$ldmfilter(verbose) == 1} {
    	lappend opts "-v";
    }

    # By default everything is sent as is
    set unzflag 0;
    if {[info exists ldmfilter($feedtype,unzflag)]} {
	set unzflag $ldmfilter($feedtype,unzflag);
    }

    set pq $ldmfilter(ldmpq);
    set status [catch {
        # prodid contains blanks
	if {$unzflag == 0} {
	    # lappend opts -q $pq -f $feed -p $prodid -s $seq $fpath;
	    # filter_ldmfeed_send $opts;
	    lappend opts -f $feed -p $prodid -s $seq $fpath;
	    filter_ldmfeed_send_pipe $opts;
	} else {
	    set fin [open "|nbspunz $fpath" r];
	    fconfigure $fin -encoding binary -translation binary;
	    set fdata [read $fin];
	    close $fin;

	    set fsize [string length $fdata];
	    fconfigure stdout -encoding binary -translation binary;

	    set fout [open "|nbsp2ldm $opts -q $pq -f $feed -p \"$prodid\" \
		-s $seq -S $fsize" w];
	    fconfigure $fout -encoding binary -translation binary;
	    puts -nonewline $fout $fdata;
	    close $fout;
	}
    } msg];

    if {$status != 0} {
        log_msg $msg;
    }
}

proc filter_ldmfeed_send {opts_list} {

    eval exec [concat nbsp2ldm $opts_list];
}

proc filter_ldmfeed_send_pipe {opts_list} {

    set status 0;
    set retry 2;
    while {$retry > 0} {
	incr retry -1;

	set status [catch {
	    if {$retry != 1} {
		::nbsp::ldm::reopen;
	    }
	    ::nbsp::ldm::push_list $opts_list;
	    ::nbsp::ldm::send;
	} errmsg];

	if {$status == 0} {
	    if {$retry == 0} {
		log_msg "Wrote to nbsp2ldm pipe";
	    }
	    break;
	} else {
	    if {$retry > 0} {
		log_msg "Could not write to nbsp2ldm: $errmsg. Retrying";
	    }
	}
    }

    if {$status != 0} {
	log_msg "Could not write to nbsp2ldm pipe.";
	catch {::nbsp::ldm::close};
    } 

    return $status;
}

proc mk_ldm_sat_prodid {rc_name} {
#
# In case of error this function returns "", which must be checked in
# the "filter_ldmfeed" function.
#
    upvar $rc_name rc;
    global ldmfilter;

    if {$ldmfilter(satdef_sourced) == 0} {
        return "";
    }

    # Extract from rc the variables that we use below, once and for all
    set fpath $rc(fpath);
    set WMOID $rc(WMOID)
    set STATION $rc(STATION);

    # This command just extracts the information from the nesdis pdb
    set status [catch {
        set params [exec nbspsatinfo -b $fpath]
    } errmsg];
    
    if {$status != 0} {
	log_msg $errmsg;
	return "";
    }

    set source [lindex $params 0];
    set creating_entity [lindex $params 1];
    set sector [lindex $params 2];
    set channel [lindex $params 3];
    set res [lindex $params 4];
    set seconds [lindex $params 5];    # unix seconds

    set ymd [clock format $seconds -gmt true -format "%Y%m%d"];
    set hm [clock format $seconds -gmt true -format "%H%M"];

    if {[info exists ldmfilter(satdef,sat_centity,$creating_entity)]} {
	set centity_name $ldmfilter(satdef,sat_centity,$creating_entity);
    } else {
	append centity_name "centity" $creating_entity;
    }
 
    if {[info exists ldmfilter(satdef,sat_sector,$sector)]} {
	set sector_name $ldmfilter(satdef,sat_sector,$sector);
    } else {
	append sector_name "sector" $sector;
    }
    
    if {[info exists ldmfilter(satdef,sat_channel,$channel)]} {
	set channel_name $ldmfilter(satdef,sat_channel,$channel);
    } else {
	append channel_name "channel" $channel;
    }

    set sat_res_suffix $ldmfilter(satdef,sat_res_suffix);

    # There are spaces here
    set prodid "satz/ch$source/$centity_name/$channel_name/$ymd";
    append prodid " $hm/$sector_name/${res}${sat_res_suffix}/";
    append prodid " " $WMOID " " $STATION;

    return $prodid;
}

# Open the nbsp2pdm pipe
set status [catch {
    set opts [list];
    # if {$ldmfilter(useseqnumid) == 1} {
    #	lappend opts "-m";
    # }
    # if {$ldmfilter(verbose) == 1} {
    #	lappend opts "-v";
    # }
    lappend opts -q $ldmfilter(ldmpq);
    eval ::nbsp::ldm::open $opts;
} errmsg];

if {$status != 0} {
    log_msg "Error opening nbsp2ldm pipe: $errmsg";
    return 1;
}

# main
main $argc $argv;

# Close the nbsp2pdm pipe
set status [catch {
    ::nbsp::ldm::close;
} errmsg];

if {$status != 0} {
    log_msg "Error closing nbsp2ldm pipe: $errmsg";
}

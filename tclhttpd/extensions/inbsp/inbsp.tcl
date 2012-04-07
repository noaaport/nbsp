#
# $Id$
#
# Functions to support the iNbsp Stats Project
#
# Usage: http://<server>[:<port>]/<command>
#
# where <command> is
#
# /_inbsp/stats[?format=xml|csv|csvk]     (status file)
# /_inbsp/missing[?format=xml|csv]   (missing file: unimplemented)
# /_inbsp/<xxx>[?format=xml|csv]     (other file: unimplemented)
#
Direct_Url /_inbsp _inbsp;

set inbsp(conf) "inbsp.conf";
set inbsp(confdir) $Config(confdir);
set inbsp(localconfdirs) $Config(localconfdirs);

# Non-configurable
set inbsp(data_format) 1;
#
## set inbsp(statusfile) $Config(nbspstatusfile);
## set inbsp(missinglogfile) $Config(missinglogfile);
## set inbsp(rtxlogfile) $Config(rtxlogfile);

# The local overrides
set _inbspconf [file join $inbsp(confdir) $inbsp(conf)];
if {[file exists ${_inbspconf}]} {
    source ${_inbspconf};
}
unset _inbspconf;

proc _inbsp/stats {{format "csvk"}} {

    global inbsp;

    # The mechanism used to return content types is to set a global
    # variable with the same name as this function.
    set _inbsp/stats "text/plain";

    if {$format eq "xml"} {
	set _inbsp/stats "text/xml";
	set type "stats";
	set r "[inbsp_output_xml_start $type]\n";
    } else {
	set r "";
    }

    append r [inbsp_output_stats $format];

    if {$format eq "xml"} {
	append r "\n[inbsp_output_xml_end $type]";
    }

    return $r;
}

proc inbsp_output_stats {format} {

    global inbsp;

    set status [catch {
	set data [exec nbspstatcounters -f $format];
    } errmsg];

    if {$status != 0} {
	return "";
    }

    # Prepend the data_format and any other metadata before the data
    # output by nbspstatcounters
    set r "";
    foreach k [list data_format] {
	if {$format eq "csv"} {
	    append r "$inbsp($k),";
	} elseif {$format eq "csvk"} {
	    append r "$k=$inbsp($k),";
	} elseif {$format eq "xml"} {
	    set r "<$k>$inbsp($k)</$k>\n";
	}
    }

    append r [string trim $data];

    return $r;
}

proc inbsp_output_xml_start {type} {

    set r "<?inbsp_${type} version=\"1.0\"?>";

    return $r;
}

proc inbsp_output_xml_end {type} {

    set r "</inbsp_${type}>";

    return $r;
}

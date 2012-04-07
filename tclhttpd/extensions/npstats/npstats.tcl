#
# $Id$
#
Direct_Url /_npstats _npstats;

set _npstats(conf) "npstats.conf";
set _npstats(localconfdirs) $Config(localconfdirs);
set _npstats(docroot) $Config(docRoot);

#
# The npstats-monitored devices
#
set _npstats(plothtdir) "npstats";
set _npstats(devices) [list];

##
## Example
##
## lappend _npstats(devices) {
##     noaaportnet.linda novra_s75.tml
## }
##

# The local overrides
set _npstatsconf [file join $Config(confdir) $_npstats(conf)];
if {[file exists ${_npstatsconf}]} {
    source ${_npstatsconf};
}
unset _npstatsconf;

proc _npstats/display_device_list {} {

    global _npstats;

    if {[llength ${_npstats(devices)}] == 0} {
	return "No devices configured.";
    }

    set result "<h1>Configured devices</h1>\n";
    append result "<ul>\n";
    foreach entry ${_npstats(devices)} {
	set deviceid [lindex $entry 0];
	set template [lindex $entry 1];
	set q "deviceid=$deviceid&template=$template";
	set url "/_npstats/display_device_status";
	append url "?" $q;
	append result "<li><a href=\"$url\">$deviceid</a></li>\n";
    }
    append result "</ul>\n";

    return $result;
}

proc _npstats/display_device_status {deviceid template} {

    global _npstats;

    set status [catch {
	set r [exec npstatspage -b \
		   -h ${_npstats(docroot)} -d ${_npstats(plothtdir)} \
		   $deviceid $template];
    } errmsg];
	
    if {$status != 0} {
	return "Cannot display status of device $deviceid";
    }

    return $r;
}

#
# $Id$
#
# This direct_url implements
#
#  _weatherscope/get/metar/daily?date=<yyyymmdd>&icao=<kkkk>
#
# Assumes that "nbspws_metartomts" is in PATH.

proc _weatherscope/get/metar/daily {date icao} {
#
# date here is yyyymmdd
#
    global _weatherscope _weatherscope/get/metar/daily;

    # The mechanism used to return content types is to set a global
    # variable with the same name as this function.
    #
    set _weatherscope/get/metar/daily $_weatherscope(metar_daily_mimetype);

    if {[regexp {(\d{4})(\d{2})(\d{2})} $date match s1 s2 s3] == 0} {
	Httpd_Error [Httpd_CurrentSocket] 400 "Invalid date";
        return "";
    }

    set arg_yyyy $s1;
    set arg_mm $s2;
    set arg_dd $s3;
    set ymd ${arg_yyyy}${arg_mm}${arg_dd};

    set result [exec nbspws-metartomts -t ${ymd} -o "-" $icao];

    # WS requires the last-modified header to properly catch the data,
    # otherwise it does not work correctly. We use the functions in
    # httpd.tcl.

    set date [clock seconds];
    Httpd_AddHeaders [Httpd_CurrentSocket] "Last-Modified: [HttpdDate $date]";

    return $result;
}

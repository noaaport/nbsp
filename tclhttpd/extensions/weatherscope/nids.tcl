#
# $Id$
#
#
# The direct_url implements
#
# _weatherscope/query/nids?....
# _weatherscope/get/nids?....
#
# We are not using
#
# Url_PrefixInstall /_weatherscope_get_nids \
#    [list _weatherscope_get_nids /_weatherscope_get_nids]
#
# and the corresponding function _weatherscope_get_nids, which would implement
#
# _weatherscope_get_nids/<suffix>  (suffix = jua/n0r/n0rjua_<ymd_hm>.nids)
#
# (Of course this would also require a change in the config.xml dataUrlFormat).
# The function appears below for documentation purposes.
#

#
# handlers
#
proc _weatherscope/query/nids {site prod start end} {

    global _weatherscope _weatherscope/query/nids;

    # The mechanism used to return content types is to set a global
    # variable with the same name as this function.
    set _weatherscope/query/nids "text/plain";

    if {[string length $site] == 4} {
	set sss [string tolower [string range $site 1 end]];
    } else {
	set sss [string tolower $site];
    }

    set flist [lsort [glob -tails -nocomplain \
	      -dir [file join $_weatherscope(rad_basedir) $sss $prod] \
	      *$_weatherscope(rad_fext)]];

    set rlist [list];
    foreach f $flist {
	set date [weatherscope_nids_get_file_date $f];
	if {([string compare $date $start] >= 0) && \
		([string compare $date $end] <= 0)} {
	    lappend rlist $f;
	}
    }

    set r "<xml>\n";
    foreach f $rlist {
	set date [weatherscope_nids_get_file_date $f];
	set yyyy [string range $date 0 3];
	set mm   [string range $date 4 5];
	set dd   [string range $date 6 7];
	set HH   [string range $date 9 10];
	set MM   [string range $date 11 12];
	append r "  <date>" "$yyyy-$mm-$dd $HH:$MM:00 UTC" "</date>\n";
    }
    append r "</xml>\n";

    return $r;
}

proc _weatherscope/get/nids {site prod fbasename} {

    global _weatherscope _weatherscope/get/nids;

    # The mechanism used to return content types is to set a global
    # variable with the same name as this function.
    #
    set _weatherscope/get/nids $_weatherscope(rad_mimetype);

    if {[string length $site] == 4} {
	set sss [string tolower [string range $site 1 end]];
    } else {
	set sss [string tolower $site];
    }

    # append to basedir "jua/n0r/n0rjua_yyyymmdd_hhmm.nids"
    set fpath [file join $_weatherscope(rad_basedir) $site $prod $fbasename];

    if {[file exists $fpath] == 0} {
	set _weatherscope/get/nids "text/plain";
	return "$basename not found.";
    }

    set status [catch {
	#
	# Canot use
	#   set content [exec nbspunz -c $_weatherscope(rad_unzskip) $fpath];
	# because exec will chop off some characters at the end even if used
	# with -keepnewline.
	#
	set F [open "|nbspunz -c $_weatherscope(rad_unzskip) $fpath" r];
	fconfigure $F -encoding binary -translation binary
	set content [read $F];
	close $F;
    } errmsg];
    if {$status != 0} {
	set _weatherscope/get/nids "text/plain";
	return $errmsg;
    }

    return $content;
}

proc weatherscope_nids_get_file_date {fbasename} {

    set date "";
    if {[regexp {_(\d{8}_\d{4})\.} $fbasename match s]} {
	set date $s
    }

    return $date;
}

#
# This function implements the Url_PrefixInstall /_weatherscope_get_nids,
# which is not being used but it is left here for documentation purposes.
#
proc _weatherscope_get_nids {prefix sock suffix} {

    global _weatherscope;

    # Check that there are no "../" constructs
    if {[regexp {\.{2}/} $suffix]} {
        Httpd_Error $sock 400 "Invalid name $suffix.";
	return;
    }

    # suffix is of the form "/jua/n0r/n0rjua_yyyymmdd_hhmm.nids"
    append fpath $_weatherscope(rad_basedir) $suffix;
    set type $_weatherscope(rad_mimetype);

    if {[file exists $fpath] == 0} {
        Httpd_Error $sock 404 "$suffix not found.";
	return;
    }

    set status [catch {
        set F [open "|nbspunz -c $_weatherscope(rad_unzskip) $fpath" r];
        fconfigure $F -encoding binary -translation binary
        set content [read $F];
        close $F;
    } errmsg];
    if {$status != 0} {
	Httpd_Error $sock 400 $errmsg;
	return;
    }

    Httpd_ReturnData $sock $type $content;
}

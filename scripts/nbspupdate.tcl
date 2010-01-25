#!%TCLSH%
#
# $Id$
#
# Usage: nbspupdate [-a <osname>-<osarch>] [-c] [-q] [-F] [<pkgname>]
#
#       If <pkgname> is not specified the default is "nbsp"
# -a => use the given <osname>-<osarch>, otherwise the hosts's parameters
# -c => just check, don't download package
# -q => quiet (default is verbose if whatever is in the conf file)
# -F => force download of pkg unconditionally
#
package require http;
package require cmdline;

proc nbspupdate_log_verbose {msg} {

    global option;

    if {$option(verbose) != 0} {
	puts $msg;
    }
}

proc nbspupdate_http_error {ht} {

    set status [::http::status $ht];

    if {$status eq "ok"} {
        set ncode [::http::ncode $ht];
        if {$ncode != 200} {
            set status 1;
            set errmsg [::http::code $ht];
        } else {
            set status 0;
        }
    } elseif {$status eq "eof"} {
        set status 1;
        set errmsg "No reply received from server.";
    } else {
        set status 1;
        set errmsg [::http::error $ht];
    }

    if {$status != 0} {
	puts $errmsg;
    }

    return $status;
}

proc nbspupdate_download_pkgindexdata {pkgindexurl} {

    set data [list];

    set ht [::http::geturl $pkgindexurl];
    set status [nbspupdate_http_error $ht];
    if {$status == 0} {
	set data [split [string trim [::http::data $ht]] "|"];
    }

    ::http::cleanup $ht;

    return $data;
}

proc nbspupdate_get_osparams {} {

    set os [exec uname];

    if {$os eq "FreeBSD"} {
	set osarch [exec uname -m];
	set osname "freebsd";
	
	return [list $osname $osarch];
    }

    if {$os ne "Linux"} {
	return [list];
    }

    set osarch [exec uname -m];
    if {[file exists "/etc/fedora-release"]} {
	set osname "fedoracore";
    } elseif {[file exists "/etc/SuSE-release"]} {
	set osname "opensuse";
    } elseif {[file exists "/etc/redhat-release"]} {
	set osname "centos";
    } elseif {[file exists "/etc/debian_version"]} {
	set osname "debian";
	if {$osarch eq "x86_64"} {
	    set osarch "amd64";
	}
    }
    
    return [list $osname $osarch];
}

proc nbspupdate_get_installedpkgversion {programversion} {

    set installed_pkgversion "";

    set status [catch {
	set p [exec which $programversion];
    } errmsg];

    if {$status == 0} {
	set installed_pkgversion [exec $programversion];
    }

    return $installed_pkgversion;
}

proc nbspupdate_download_pkg {pkgfilename pkgurl} {

    set F [open $pkgfilename "w"];
    fconfigure $F -translation binary -encoding binary;
    set ht [::http::geturl $pkgurl -binary 1 -channel $F];
    close $F;

    set status [nbspupdate_http_error $ht];
    if {$status == 1} {
	file delete $pkgfilename;
    }

    ::http::cleanup $ht;

    return $status;
}

#
# main
#
source "/usr/local/etc/nbsp/nbspupdate.conf";

set usage {usage: nbspupdate [-a <osname>-<osarch>] [-c] [-F] [<pkgname>]};
set optlist {{a.arg ""} {c} {q} {F}};
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

## Check for option conflict
if {($option(c) == 1) && ($option(F) == 1)} {
    puts "Options -c and -F conflict.";
    return 1;
}

# The program name can be overriden in the cmd line
if {$argc != 0} {
    set pkgname [lindex $argv 0];
}

## Get the default os params
set osparams [nbspupdate_get_osparams];
set osname [lindex $osparams 0];
set osarch [lindex $osparams 1];

## Check override in the cmd line
if {$option(a) ne ""} {
    if {[regexp {(.+)-(.+)} $option(a) match osname osarch] == 0} {
	puts "Invalid value of option -a: $option(a)";
	return 1;
    }
}

## Verbose
if {$option(q) == 0} {
    set option(verbose) $verbose;
} else {
    set option(verbose) 0;
}

## Construct the url to the pkg index file for this os/arch
set pkgindexurl "$pkgindexbaseurl/$pkgname-$osname-$osarch";

## The name of the program that gives the program version
append programversion ${pkgname} "version";

## Download the index file and get the data for the latest version available
nbspupdate_log_verbose "Downloading index: $pkgindexurl ...";
#
set data [nbspupdate_download_pkgindexdata $pkgindexurl]
if {[llength $data] == 0} {
    return 0;
}
#
set pkgversion [lindex $data 1];
set pkgurl [lindex $data 2];
#
set pkgfilename [file tail $pkgurl];
set pkgfilerootname [file rootname $pkgfilename];
regexp "$pkgname-(.+)" $pkgfilerootname match new_pkgversion;

# Get the installed version and compare with the new_pkgversion that
# was obtained above
set installed_pkgversion [nbspupdate_get_installedpkgversion $programversion];

#
#### This a test
#### set installed_pkgversion "0.5.p1.9_1";
#

if {$installed_pkgversion eq $new_pkgversion} {
    puts "Latest version $new_pkgversion installed.";
    if {$option(F) == 0} {
	return 0;
    }
} else {
    puts "Installed = $installed_pkgversion / Latest = $new_pkgversion";
}

if {$option(c) == 1} {
    return 0;
}

nbspupdate_log_verbose "Downloading $pkgfilename ...";
set status [nbspupdate_download_pkg $pkgfilename $pkgurl];
if {$status == 0} {
    nbspupdate_log_verbose "Done";
}

#!%TCLSH%
#
# $Id$
#
# Usage: nbspupdate [-a <osname>-<osarch>] [-c] [-k] [-q] [-u] [-F] [<pkgname>]
#
#       If <pkgname> is not specified the default is "nbsp"
# -a => Use the given <osname>-<osarch>, otherwise the hosts's parameters
# -c => Just check, don't download package
# -k => If -u is given, keep (do not delete) the package file 
# -q => Quiet (default is verbose if whatever is in the conf file)
# -u => Upgrade (pkg_delete and pkg_add)
# -F => Force download (or install) of pkg unconditionally
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

proc nbspupdate_get_filepkgversion {pkgname pkgfilename} {
#
# Extract the version from the package file name
#
    set pkgfext [string range [file extension $pkgfilename] 1 end];
    set pkgfilerootname [file rootname $pkgfilename];

    # For rpm and deb, cut out the arch
    if {$pkgfext eq "tbz"} {
	regexp "${pkgname}-(.+)" $pkgfilerootname match new_pkgversion;
    } elseif {$pkgfext eq "rpm"} {
	set pkgfilerootname [file rootname $pkgfilerootname];
	regexp "${pkgname}-(.+)" $pkgfilerootname match new_pkgversion;
    } elseif {$pkgfext eq "deb"} {
	set i [string last "_" $pkgfilerootname];
	incr i -1;
	set pkgfilerootname [string range $pkgfilerootname 0 $i];
	regexp "${pkgname}_(.+)" $pkgfilerootname match new_pkgversion;
    }

    return $new_pkgversion;
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

proc nbspupdate_upgrade_pkg {osname pkgname pkgfilename} {

    global option;

    if {$osname eq "freebsd"} {
	set delcmd [list pkg_delete "$pkgname-*"];
	set addcmd [list pkg_add $pkgfilename];
    } elseif {$osname eq "centos"} {
	set delcmd [list rpm -e $pkgname];
	set addcmd [list rpm -i $pkgfilename];
    } elseif {$osname eq "debian"} {
	set delcmd [list dpkg -r $pkgname];
	set addcmd [list dpkg -i $pkgfilename];
    } else {
	return -code error "Invalid osname: $osname";
    }
	
    set status [catch {
	eval exec $delcmd;
    } errmsg];

    # If there is an error deleting the package, put an error message and
    # return, unless -F was given.
    if {$status != 0} {
	if {$option(F) == 0} {
	    puts $errmsg;
	    return 1;
	}
    }

    set status [catch {
	eval exec $addcmd;
    } errmsg];

    if {$status != 0} {
	puts $errmsg;
    }

    return $status;
}

#
# main
#
source "/usr/local/etc/nbsp/nbspupdate.conf";

set usage {usage: nbspupdate [-a <osname>-<osarch>] [-c] [-k] [-q] [-u]
[-F] [<pkgname>]};
set optlist {{a.arg ""} {c} {k} {q} {u} {F}};
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

## Check for option conflict
if {($option(c) == 1) && (($option(F) == 1) || ($option(u) == 1))} {
    puts "Options -c and -F|-u conflict.";
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
set pkgversion [lindex $data 1];  # does not contain the package build number 
set pkgurl [lindex $data 2];
#
set pkgfilename [file tail $pkgurl];

# Get the installed version and the full (including the build number) new
# version and then compare.
set new_pkgversion [nbspupdate_get_filepkgversion $pkgname $pkgfilename];
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
} else {
    return 1;
}

if {$option(u) == 0} {
    return;
}

nbspupdate_log_verbose "Upgrading $pkgname ...";

set status [nbspupdate_upgrade_pkg $osname $pkgname $pkgfilename];

if {$option(k) == 0} {
    file delete $pkgfilename;
}

if {$status != 0} {
    return 1;
}

nbspupdate_log_verbose "Done";

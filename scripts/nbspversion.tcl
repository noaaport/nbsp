#!%TCLSH%
#
# $Id$

set pkgname "nbsp";

if {[file executable "/usr/sbin/pkg_info"]} {
    set output [string trim \
		    [lindex [split [exec pkg_info | grep "${pkgname}-"]] 0]];
    regexp ${pkgname}-(.+) $output match version;
} elseif {[file executable "/usr/bin/dpkg"]} {
    set output [exec dpkg -s ${pkgname} | grep -m 1 "Version"];
    set version [string trim [lindex [split $output] 1]];
} elseif {[file executable "/bin/rpm"]} {
    set output [exec rpm -qa | grep "${pkgname}-"];
    regexp ${pkgname}-(.+) $output match version;
} else {
    set version "unknown";
}

puts $version;

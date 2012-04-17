#!%TCLSH%
#
# $Id$
#
# In the case of deb and rpm packages, the version string output here
# does not include the arch portion (this is also the convemtion sued
# in nbspupdate). In the rpm's, we also cut the ".el<n>" portion. 
#
# The version string output here includes the package build number.

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
    # Cut the arch and "el<n>"
    set version [file rootname [file rootname $version]];
} else {
    set version "unknown";
}

puts $version;

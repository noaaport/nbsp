#
# $Id$
#

## The common defaults
set defaultsfile "/usr/local/etc/nbsp/filters.conf";
if {[file exists $defaultsfile] == 0} {
   log_err "$defaultsfile not found.";
}
source $defaultsfile;

if {[file exists $gempak(envfile)] == 0} {
    log_err "$gempak(envfile) not found.";
}
source $gempak(envfile);

# Packages from tcllib
package require cmdline;

# Nbsp packages
## The errx library. syslog enabled below if -b is given.
package require nbsp::errx;
package require nbsp::util;

# Common (optional) configuration  file
set nbspradmos(conf)      [file join $common(confdir) "nbspradmos.conf"];

# nbspradmos defaults
set nbspradmos(localconfdirs) $common(localconfdirs);
#
set nbspradmos(rcsubdir)  [file join "gdplot2" "rad"];
set nbspradmos(rcfile)    "radmos.n0r.rc";
set nbspgdradr(Cdir)      "/var/noaaport/gempak/images/rad";
#
set gdplot2(devfmt) "gif";
set gdplot2(devsize) "";

# nbspgdradr defaults
set nbspgdradr(localconfdirs) $common(localconfdirs);
#
set nbspgdradr(rcsubdir)  "gdradr";
set nbspgdradr(rcfile)    "gdradr.rc";
set nbspgdradr(namefmt)   "%Y%m%d_%H%M.gem";
set nbspgdradr(Cdir)      "/var/noaaport/gempak/nexrad";

# optional config file
if {[file exists $nbspradmos(conf)]} {
    source $nbspradmos(conf);
}

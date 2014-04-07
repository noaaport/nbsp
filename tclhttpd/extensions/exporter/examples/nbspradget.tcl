#!/usr/bin/tclsh
#
# $Id$
#
# Usage:
#  nbspradget [-h <host>] [-l | -n <n>] [-p <port>] [-s <awips1>] [-t <awips2>]
#
# -l => print the list of files available and return (-n is ignored)
# -n => download the most recent n files (default 1)
# -s => radar site (default jua)
# -t => type (default n0r)
# -h => host (default www.opennoaaport.net)
# -p => port (default 8015)
#
# Exit codes:
#  0 => ok
#  1 => invalid options
#  2 => no files available in host
#
package require cmdline;

set usage {nbspradget [-h <host>] [-l | -n <n>] [-p <port>]
[-s <awips1>] [-t <awips2>]};

set optlist {{h.arg ""} l {n.arg ""} {p.arg ""} {s.arg ""} {t.arg ""}};

set g(dir_t) {digatmos/nexrad/nids/$g(awips2)/$g(awips1)};
set g(queryurl_t) \
    {http://$g(host):$g(port)/_export/query_dir?dir=$g(dir)&select=$g(select)};
set g(geturl_t) {http://$g(host):$g(port)/$g(dir)/$g(file)};

# defaults
set g(host) "www.opennoaaport.net";
set g(port) "8015";
#
set g(awips1) "n0r";
set g(awips2) "jua";
#
set g(nfiles) 1;    # one file (the last one)
set g(select) "\.nids\$";
set g(file) "latest";

#
# main
#
array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];

if {$option(h) ne ""} {
    set g(host) $option(h);
}

if {$option(n) ne ""} {
    set g(nfiles) $option(n);
}

if {$option(p) ne ""} {
    set g(port) $option(p);
}

if {$option(s) ne ""} {
    set g(awips2) $option(s);
}

if {$option(t) ne ""} {
    set g(awips1) $option(t);
}

# Validate options
if {$g(nfiles) < 1} {
    return 1;  # invalid input
}

foreach k [list dir queryurl geturl] {
    set k_t ${k}_t;
    set g($k) [subst $g($k_t)];
}

# set output [exec wget -q -O - $g(queryurl)];
# exec wget -q -O $g(file) $g(geturl);

set output_list [lsort -decreasing [split [exec curl -s -S $g(queryurl)]]];

# If -l is given, output the list and return
if {$option(l) == 1} {
    puts [join $output_list "\n"];
    return 0;
}

# Save the number of files available in nfiles
set nfiles [llength $output_list];

# If there are no files, return with an error
if {$nfiles == 0} {
    return 2;   # no files available
}

# If the requested number is greater than the number of available files
# truncate the requested number to the maximum.
if {$g(nfiles) > $nfiles} {
    set g(nfiles) $nfiles;
}

incr g(nfiles) -1;

foreach f [lrange $output_list 0 $g(nfiles)] {
    set g(file) $f;
    set g(geturl) [subst $g(geturl_t)];
    exec curl -s -S -o $g(file) $g(geturl);
}

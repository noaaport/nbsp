#!%TCLSH%
#
# $Id$

# Example(s):
#
# nbspmtrcsvd [-t] <spooldir>/tjsj_saus42-mtrsju.192357_9116329
# nbspmtrcsvd [-t] <datadir>/2007042000.sao
#
# Assumes that the programs "nbspmtrd" and "nbspmtrcsv" are in PATH.
# Uses nbspmtrcsv to extract the data records, and nbspmtrd to decode them.
# Without [-t], prints the report in human readable form; with [-t] in tabular
# form.
#
package require cmdline;

set usage {Usage: nbspmtrcsvd [-t] <file>};
set optlist {{t}};

set mtr2csv "nbspmtrcsv";
set csv2rep "nbspmtrd";

array set option [::cmdline::getoptions argv $optlist $usage];
set argc [llength $argv];
if {$argc != 1} {
    puts $usage;
    exit 1;
}
set inputfile [lindex $argv 0];

if {[file exists $inputfile] == 0} {
    puts "$inputfile not found.";
    exit 1;
}

if {$option(t) == 1} {
    set result [exec $mtr2csv "," $inputfile | $csv2rep -c -t];
} else {
    set result [exec $mtr2csv "," $inputfile | $csv2rep -c];
}
puts $result;

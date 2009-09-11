#!/usr/local/bin/tclsh8.4

set nbspsatbin "/usr/local/bin/nbspsat";
set gpmaprc     "satmap.rc";
set gpenvrc	"gpenv.rc";
set gpmapbin	"/home/gempak/bin/freebsd/gpmap";
set devsize	"300;300";
set devtype	"png";

if {$argc == 0} {
    puts "Needs one argument.";
    return;
}

set fpath [lindex $argv 0];

source $gpenvrc

set params [exec $nbspsatbin -g $fpath]
set sector [lindex $params 2]
set channel [lindex $params 3]
set res [lindex $params 4]
set time [lindex $params 5]
set infile [lindex $params 6]

set outfile "$infile.$devtype";
append logfile $infile ".log";

file delete $outfile

set fin [open $gpmaprc r];
set fout [open "|$gpmapbin > $logfile" w];
fconfigure $fin -translation binary -encoding binary;
fconfigure $fout -translation binary -encoding binary;
set script [read $fin]
puts $fout [subst $script]
close $fin;
close $fout;

exec /home/gempak/bin/freebsd/gpend;

file delete gemglb.nts last.nts $infile
puts [lreplace $params 6 6 $outfile]







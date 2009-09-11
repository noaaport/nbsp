#!/usr/bin/perl
#
# $Id$
#
# Sample script for logging all fils procesed.

use File::Basename;
use File::Path;
use Sys::Syslog;
use strict;

our $glogdir = "/var/noaaport";

sub main() {

    my $line;
    my ($seq, $type, $cat, $code, $fpath);
    my ($fname, $dirname);

    openlog "nbsp-logger", "ndelay", "nowait", "user"; 

    if(open(STDOUT, ">$glogdir/list.log") == undef){
	syslog("err", "Could not open logfile. $!");
	closelog;
	exit;
    }
    
    while($line = <STDIN>){
	print(STDOUT $line);
	chop($line);
	($seq, $type, $cat, $code, $fpath) = split(/\s+/, $line);
	$dirname = dirname($fpath);
	$fname = basename($fpath);
#	print(STDOUT "$seq $fname\n");
    }

    closelog;
}


main();


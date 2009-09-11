#!/usr/bin/perl
#
# $Id$
#
# This script is installed as an inn "program feed" to retrieve each
# article and store it in an nbsp compatible directory structure.

use File::Basename;
use File::Path;
use Sys::Syslog;
use strict;

our $gverbose = 1;

our $gdatadir = "/var/noaaport/data/inn";
our $SM = "/usr/local/news/bin/sm";

our $ggempak_filter = "/var/noaaport/filters/gempak-filter.pl";
our $ggempak_filter_opts = "";

sub main() {

    my $line;
    my $finfo;
    my $token;
    my $dummy;

    if($#ARGV == -1){
	return;
    }
    
    openlog "rcvfrominn", "ndelay", "nowait", "user"; 

    $token = $ARGV[0];

    if(open(FIN, "$SM $token|") == undef){
        syslog("err", "Could not open $SM: $!");
    }else{
	# Keep reading until the blank line. The next line contains our
	# fname, seq header.
	$line = <FIN>;
	while($line !~ /^\s+$/){
	    $line = <FIN>;
	}

	$line = <FIN>;
	if(substr($line, 0, 1) eq "["){
	    $finfo = substr($line, 1, -2);
	    process_product($finfo);
	}
	close(FIN);
    }

    closelog;
}

sub process_product(){

    my ($finfo) = @_;
    my ($seq, $type, $cat, $code, $fname);
    my $fpath;
    my $line;
    my ($station, $wmoid, $awips, $notawips, $type);
    my ($awips1, $awips2);
    my $savedir;
    my $status;

    ($seq, $type, $cat, $code, $fname) = split(/\s+/, $finfo);

    # The file name is of the form "tjsj_sdus52-ncrjua.4" 
    # when there is an awips code, or "kwbc_ytqm52+grib.4".
    # In some cases, the third component is absent as well.

    if($fname =~ /-/){
	$notawips = "";
	($station, $wmoid, $awips, $type) = split(/[_.\-]/, $fname);
	$awips1 = substr($awips, 0, 3);
	$awips2 = substr($awips, 3);
    }elsif($fname =~ /\+/){
	$awips = "";
	$awips1 = "";
	$awips2 = "";
	($station, $wmoid, $notawips, $type) = split(/[_+.\-]/, $fname);
    }else{
	$awips = "";
	$awips1 = "";
	$awips2 = "";
	$notawips = "";
	($station, $wmoid, $type) = split(/[_.]/, $fname);
    }    

    $savedir = "$gdatadir/\U$station";
    $fpath = "$savedir/$fname";

    if(-e $savedir && !-d $savedir){
	syslog("err", "Exists and not dir.");
	return;
    }elsif(!-e $savedir){
	if(mkpath($savedir, 0, 0755) == 0){
	    syslog("err", "Cannot make $savedir: $!");
	    return;
	}
    }

    $status = open(FOUT, ">$fpath");
    if($status == undef){
	syslog("err", "Could not open $fpath: $!");
	return;
    }

    while($line = <FIN>){
	print(FOUT $line);
    }

    close(FOUT);

    if($gverbose == 1){
	syslog("info", "Retrieving $fname from inn.");
    }

    # We can now pass this to the gempak filter
    $finfo = "$seq $type $cat $code $fpath";

    if(open(FOUT, "|$ggempak_filter $ggempak_filter_opts") == undef){
	syslog("err", "Could not open $ggempak_filter: $!");
    }else{
	syslog("info", "Feeding $ggempak_filter: $finfo");
	print(FOUT "$finfo\n");
	close(FOUT);
    }
}

main();

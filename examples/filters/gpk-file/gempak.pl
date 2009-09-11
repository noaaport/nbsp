#!/usr/bin/perl
#
# $Id$
#
# Sample script for storing files in a gempak-like directory tree. 
# This file can be placed in the nbsp dev directory, and then a HUP
# signal to nbspd (if it already running) to load it. The only
# configurable option is the location of the root of the tree
# specified in the variable "$gdatadir" below.  

use File::Basename;
use File::Path;
use Compress::Zlib;
use Sys::Syslog;
use strict;

our $gdatadir = "/var/noaaport/gempak";
our $gverbose = 1;
our $glogfile = 0;

our $gmpk_header_fmt = "\001\r\r\n%03d \r\r\n"; 
our $gmpk_trailer_str = "\r\r\n\003"; 

sub main() {

    my $line;
    my ($seq, $type, $cat, $code, $fpath);
    my ($fname, $dirname);

    openlog "gp-filter", "ndelay", "nowait", "daemon"; 

    if($glogfile == 1){
	if(open(STDOUT, ">/var/noaaport/gempak-filter.log") == undef){
	    syslog("err", "Could not open logfile. $!");
	    $glogfile = 0;
	}
    }

    while($line = <STDIN>){

	if($glogfile == 1){
	    print $line;
	}

	chop($line);
	($seq, $type, $cat, $code, $fpath) = split(/\s+/, $line);

	$dirname = dirname($fpath);
	$fname = basename($fpath);

	process_product($seq, $fpath, $fname);
    }

    if($glogfile == 1){
	close(STDOUT);
    }

    closelog;
}

sub process_product(){

    my ($seq, $fpath, $fname) = @_;
    my ($station, $wmoid, $awips, $notawips, $type);
    my ($awips1, $awips2);
    my @time;
    my $ymdh;
    my $ymd_hm;
    my $savedir;
    my $savename;
    my $f_append = 1;
    my $f_compress = 0;
    my $status;
    my $line1;
    my $line2;
    my $subline1;
    my @body;
    my $body;
    my $cbody;
    my $framenumber;
    my $size;
    my $matchfound = 0;

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

    @time = gmtime();
    $time[5] += 1900;
    $time[4] += 1;
    $ymdh = sprintf("%d%02d%02d%02d", $time[5], $time[4], $time[3], $time[2]); 
    $ymd_hm = sprintf("%d%02d%02d_%02d%02d", 
		      $time[5], $time[4], $time[3], $time[2], $time[1]); 

    $matchfound = 0;


if($awips1 =~ /cfw|cwf|glf|gls|hfs|ice|lsh|maw|mws|nsh|off|omr|osw|pls|smw|srf|tid/){
	$savedir = "nwx/marine/\U$awips1";
	$savename = "$ymdh.\U$awips1";
	
	$matchfound = 1;
}
elsif($awips1 =~ /afd|afm|afp|aws|awu|ccf|lfp|lsr|mis|now|opu|pns|rec|rer|rtp|rws|rzf|scc|sfp|sft|sls|sps|stp|swr|tav|tpt|tvl|wcn|wvm|zfp/){
	$savedir = "nwx/pub_prod/\U$awips1";
	$savename = "$ymdh.\U$awips1";
	
	$matchfound = 1;
}
elsif($awips1 =~ /chg|dsa|hls|psh|tca|tcd|tce|tcm|tcp|tcs|tcu|tma|tsm|tsu|twd|two|tws/){
	$savedir = "nwx/tropical/\U$awips1";
	$savename = "$ymdh.\U$awips1";
	
	$matchfound = 1;
}
elsif($awips1 =~ /npw|svr|svs|tor|wsw|flw|ffw/){
	$savedir = "nwx/watch_warn/\U$awips1";
	$savename = "$ymdh.\U$awips1";
	
	$matchfound = 1;
}
elsif($awips1 =~ /n0.|n1p|n[a-z].|n[1-3][rsvz]/){
	$savedir = "nexrad/NIDS/\U$awips2/\U$awips1";
	$savename = "\U${awips1}_$ymd_hm";
	$f_append = 0; $f_compress = 1;
	$matchfound = 1;
}
elsif($awips =~ /fwddy(.)/){
	$savedir = "nwx/fire/fwd";
	$savename = "$ymdh.fwddy$1";
	
	$matchfound = 1;
}
elsif($awips =~ /swody([0-9])/){
	$savedir = "nwx/spc/day$1";
	$savename = "$ymdh.day$1";
	
	$matchfound = 1;
}
elsif($awips =~ /ptsdy([0-9])/){
	$savedir = "nwx/spc/day$1";
	$savename = "$ymdh.ptsdy$1";
	
	$matchfound = 1;
}
elsif($wmoid =~ /wwus20/){
	$savedir = "nwx/spc/watch";
	$savename = "$ymdh.watch";
	
	$matchfound = 1;
}
elsif($wmoid =~ /wwus30/){
	$savedir = "nwx/spc/watch";
	$savename = "$ymdh.wtch2";
	
	$matchfound = 1;
}
elsif($awips1 =~ /wou/){
	$savedir = "nwx/spc/wou";
	$savename = "$ymdh.wou";
	
	$matchfound = 1;
}
elsif($wmoid =~ /wous40/){
	$savedir = "nwx/spc/public";
	$savename = "$ymdh.public";
	
	$matchfound = 1;
}
elsif($wmoid =~ /nwus2[02]/){
	$savedir = "nwx/spc/svr_summ";
	$savename = "$ymdh.svr";
	
	$matchfound = 1;
}
elsif($awips =~ /stahry/){
	$savedir = "nwx/spc/stahry";
	$savename = "$ymdh.hry";
	
	$matchfound = 1;
}
elsif($awips =~ /stadts/){
	$savedir = "nwx/spc/stadts";
	$savename = "$ymdh.dts";
	
	$matchfound = 1;
}
elsif($wmoid =~ /acus11/){
	$savedir = "nwx/spc/meso";
	$savename = "$ymdh.meso";
	
	$matchfound = 1;
}
elsif($wmoid =~ /wwus44/){
	$savedir = "nwx/spc/hzrd";
	$savename = "$ymdh.hzrd";
	
	$matchfound = 1;
}
elsif($wmoid =~ /wous20/){
	$savedir = "nwx/spc/status";
	$savename = "$ymdh.stat";
	
	$matchfound = 1;
}
elsif($awips =~ /sev[0-9]/){
	$savedir = "$gdatadir/nwx/spc/sev";
	$savename = "$ymdh.sev";
	
	$matchfound = 1;
}
elsif($awips =~ /sevspc/){
	$savedir = "nwx/spc/sevmkc";
	$savename = "$ymdh.sevmkc";
	
	$matchfound = 1;
}
elsif($wmoid =~ /fvxx2[0-4]/){
	$savedir = "nwx/volcano/volcano";
	$savename = "$ymdh.volc";
	
	$matchfound = 1;
}
elsif($wmoid =~ /fvcn0[0-4]/){
	$savedir = "nwx/volcano/volcano";
	$savename = "$ymdh.volc";
	
	$matchfound = 1;
}
elsif($wmoid =~ /fvau0[0-4]/){
	$savedir = "nwx/volcano/volcano";
	$savename = "$ymdh.volc";
	
	$matchfound = 1;
}
elsif($wmoid =~ /wv/){
	$savedir = "nwx/volcano/volcwarn";
	$savename = "$ymdh.vlcw";
	
	$matchfound = 1;
}
elsif($wmoid =~ /fvus2[01]/){
	$savedir = "nwx/volcano/volcfcst";
	$savename = "$ymdh.vlcf";
	
	$matchfound = 1;
}
elsif($awips1 =~ /agf|ago|fwl|saf|wcr|wda/){
	$savedir = "nwx/ag_prod/\U$awips1";
	$savename = "$ymdh.\U$awips1";
	
	$matchfound = 1;
}
elsif($awips1 =~ /ava|avm|avw|sab|sag|sew|wsw/){
	$savedir = "nwx/avalanche/\U$awips1";
	$savename = "$ymdh.\U$awips1";
	
	$matchfound = 1;
}
elsif($awips1 =~ /aww|oav|rfr|sad|sam|sig|wst|wsv/){
	$savedir = "nwx/aviation/\U$awips1";
	$savename = "$ymdh.\U$awips1";
	
	$matchfound = 1;
}
elsif($awips1 =~ /fa[0-9]/){
	$savedir = "nwx/aviation/area";
	$savename = "$ymdh.area";
	
	$matchfound = 1;
}
elsif($awips1 =~ /wa[0-9]/){
	$savedir = "nwx/aviation/airmet";
	$savename = "$ymdh.airm";
	
	$matchfound = 1;
}
elsif($awips1 =~ /ws[0-9]/){
	$savedir = "nwx/aviation/sigmet";
	$savename = "$ymdh.sgmt";
	
	$matchfound = 1;
}
elsif($awips1 =~ /cli|clm|cmm/){
	$savedir = "nwx/climate/\U$awips1";
	$savename = "$ymdh.\U$awips1";
	
	$matchfound = 1;
}
elsif($awips1 =~ /fdi|frw|fwa|fwe|fwf|fwm|fwn|fwo|fws|fww|pbf|rfd|rfw|smf/){
	$savedir = "nwx/fire/\U$awips1";
	$savename = "$ymdh.\U$awips1";
	
	$matchfound = 1;
}
elsif($awips1 =~ /esf|ffa|ffg|ffh|ffs|ffw|fln|fls|flw/){
	$savedir = "nwx/flood/\U$awips1";
	$savename = "$ymdh.\U$awips1";
	
	$matchfound = 1;
}
elsif($awips =~ /pmd(ca|hi|sa|epd|ep[3-7]|spd|thr|hmd)|preepd/){
	$savedir = "nwx/misc/\U$awips";
	$savename = "$ymdh.\U$awips";
	
	$matchfound = 1;
}
elsif($awips =~ /qpf(ptr|rsa|str)/){
	$savedir = "nwx/qpf/QPF";
	$savename = "$ymdh.QPF";
	
	$matchfound = 1;
}
elsif($awips =~ /qpferd|qpfhsd|qpfpfd/){
	$savedir = "nwx/qpf/\U$awips";
	$savename = "$ymdh.\U$awips";
	
	$matchfound = 1;
}
elsif($awips =~ /qps/){
	$savedir = "nwx/qpf/QPS";
	$savename = "$ymdh.QPS";
	
	$matchfound = 1;
}
elsif($awips1 =~ /rva|rvd|rvf|rvi|rvm|rvr|rvs/){
	$savedir = "nwx/river/\U$awips1";
	$savename = "$ymdh.\U$awips1";
	
	$matchfound = 1;
}
elsif($awips1 =~ /scp|scv|sim/){
	$savedir = "nwx/satellite/\U$awips1";
	$savename = "$ymdh.\U$awips1";
	
	$matchfound = 1;
}

    if($matchfound == 0){
	next;
    }

    if(-e $savedir && !-d $savedir){
	syslog("err", "Exists and not dir.");
	next;
    }elsif(!-e $savedir){
	if(mkpath($savedir, 0, 0755) == 0){
	    syslog("err", "Cannot make $savedir: $!");
	    next;
	}
    }

    if(open(FIN, $fpath)  == undef){
	syslog("err", "Could not open $fpath: $!");
	next;    
    }else{
	if($f_append == 1){
	    $status = open(FOUT, ">>$savedir/$savename");
	}else{
	    $status = open(FOUT, ">$savedir/$savename");
	}
	if($status == undef){
	    close(FIN);
	    syslog("err", "Could not open $savedir/$savename: $!");
	    next;
	}
    }

    printf(FOUT $gmpk_header_fmt, $seq % 1000);
    if($f_compress == 1){
	if($gverbose == 1){
	    syslog("info", "Compressing $savename.");
	}

	# For gempak compatibility, the entire product must be split
	# in 4000 bytes frames (as in the raw noaaport), then compress
	# each frame individually, and then catenate the compressed
	# frames. That is prepended with the wmo and awips header 
	# (from the the first and second lines of the original uncompressed
	# file). Furthermore, the frames must be compressed with level 9
	# and with the compress() function, not with ->deflate.

	$line1 = <FIN>;
	$size = length($line1);
	$subline1 = substr($line1, 24);
	print(FOUT $subline1);
	$line2 = <FIN>;
	$size += length($line2);
	print(FOUT $line2);
	$framenumber = 0;
	while(read(FIN, $body, 4000 - $size) > 0){
	    if($framenumber == 0){
		++$framenumber;
		$size = 0;	# read full 4000 bytes for ther frames
		$body = join("", $line1, $line2, $body);
	    }
	    $cbody = compress($body, 9);
	    print(FOUT $cbody);
	}
    }else{
	$line1 = <FIN>;
	@body = <FIN>;
	$line1 = substr($line1, 24);
	print(FOUT $line1);
	print(FOUT @body);
    }
    printf(FOUT $gmpk_trailer_str);

    close(FIN);
    close(FOUT);

    if($gverbose == 1){
	syslog("info", "Archiving $fname in $savedir/$savename.");
    }
}

main();


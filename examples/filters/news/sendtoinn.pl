#!/usr/bin/perl
#
# $Id$
#
# This script feeds each product to inn.

use File::Basename;
use File::Path;
use Sys::Syslog;
use Net::NNTP;
use strict;

our $gverbose = 0;
our $gfrom = "From: nbspfeed\@noaaport.net";
our $gnewsserver = "diablo";

sub main() {

    my $finfo;
    my ($seq, $type, $cat, $code, $fpath);
    my ($fname, $dirname);

    openlog "nbspnewsfeed", "ndelay", "nowait", "user"; 
    
    while($finfo = <STDIN>){

	chop($finfo);

	if($finfo eq ""){
	    syslog("info", "Received request to quit.");
	    last;
	}

	($seq, $type, $cat, $code, $fpath) = split(/\s+/, $finfo);

	$dirname = dirname($fpath);
	$fname = basename($fpath);

	$finfo = "$seq $type $cat $code $fname";
	process_product($finfo, $fpath, $fname);
    }

    closelog;
}

sub process_product(){

    my ($finfo, $fpath, $fname) = @_;
    my ($station, $wmoid, $awips, $notawips, $type);
    my ($awips1, $awips2);
    my $news_group;
    my $news_subject;
    my @header;

    # The file name is of the form "tjsj_sdus52-ncrjua.<seq>" 
    # when there is an awips code, or "kwbc_ytqm52+grib.<seq>".
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

    if($awips eq ""){
	return;
    }

    $news_group = "Newsgroups: noaaport.$station";
    $news_subject = "Subject: $awips";

    if($awips1 =~ /n0r|n0s|n0v|n0z|n1p|n1r|n1s|n1v|n2r|n2s|n2v|n3p|n3r|n3s|n3v|nco|ncr|ncz|net|nhl|nla|nll|nml|ntp|nup|nvl|nvw/){
#
# radar
#
	return;
    }

    if($awips1 =~ /hd1|hd2|hd3|hd4|hd5|hd6|hd7|hd8|hd9|hp1|hp2|hp3|hp4|hp5|hp6|hp7|hp8|hp9/){
#
# data
#
	return;
    }

    if($awips1 =~ /rbg/){
	# redbook graphics
	return;
    }

    if($awips1 =~ /18a|24a|30l|3hr|5tc|abv|ada|adm|adr|adv|adw|adx|afd|afm|afp|agf|ago|apg|alt|aqi|arp|asa|avm|awg|aws|awu|aww|boy|ccf|cem|cfw|cgr|chg|cli|clm|cmm|cod|csc|cur|cwa|cwf|cws|day|dpa|dsa|dsm|dst|efp|eol|eon|eqr|esf|esg|esp|ess|faa|fa0|fa1|fa2|fa3|fa4|fa5|fa6|fa7|fa8|fa9|fak|fan|fav|fbp|fd0|fd1|fd2|fd3|fd8|fd9|fdi|ffa|ffg|ffh|ffs|ffw|fln|fls|flw|fmr|fof|foh|frh|fsh|fss|ftj|ftm|ftp|fwa|fwc|fwd|fwe|fwf|fwl|fwm|fwn|fwo|fws|fzl|glf|glo|gls|gsm|hcm|hdp|hls|hmd|hsf|hff|hwo|hyd|hym|hyw|ice|ini|iob|kpa|law|lco|lfp|lls|lsh|lsr|ltg|man|map|mav|maw|mef|mex|mim|mis|mon|mrm|mrp|msm|mst|mtr|mtt|mvf|mws|now|npw|nsh|oav|ocd|ofa|off|omr|opu|osb|oso|osw|oua|par|pbf|pib|pir|pls|pmd|pns|prc|psh|psm|pvm|pwo|qcd|qch|qcm|qcw|qpf|qps|rcm|rdf|rdg|rec|rep|rer|rfd|rfr|rfw|rob|rr1|rr2|rr3|rr4|rr5|rr6|rr7|rr8|rr9|rra|rrc|rrm|rrs|rrx|rry|rsd|rsm|rtp|rva|rvd|rvf|rvi|rvm|rvr|rvs|rws|rzf|saa|sab|sad|saf|sag|sam|saw|scc|scd|scn|scp|scs|scv|sdo|sds|sel|ses|sev|sfd|sfp|sft|sgl|sgw|shi|shn|shp|sig|sim|sls|sma|smf|smw|spe|spf|sps|srf|ssa|ssi|ssm|sta|std|sto|stp|stw|svr|svs|swd|swe|swo|swr|sws|taf|tap|tav|tca|tcd|tce|tcm|tcp|tcs|tcu|tid|tma|tor|tpt|tsm|tst|tsu|tvl|twb|twd|het|hwd|hnd|two|tws|ujx|ulg|uvi|vaa|ver|vgp|wa0|wa1|wa2|wa3|wa4|wa5|wa6|wa7|wa8|wa9|wac|war|swx|wat|wcn|wcr|wda|wek|wpd|wrk|ws1|ws2|ws3|ws4|ws5|ws6|ws7|ws8|ws9|wst|wsv|wsw|wts|wvm|wwa|zfp|sum|int|adm|afd|air|apt|ash|aws|cem|cfw|cht|cli|clm|cwf|dy1|dy2|eln|ema|eml|eph|eqr|esf|ess|faa|fee|ffa|ffs|ffw|fln|flw|fwf|glf|glo|gls|haa|had|haf|ham|hap|has|hat|haw|hea|hed|hef|hem|hep|hes|het|hew|hff|hls|hna|hnd|hnf|hnm|hnp|hns|hnt|hnw|hsa|hsd|hsf|hsm|hsp|hst|hss|hsw|hwa|hwd|hwf|hwm|hwp|hws|hwt|hwu|hww|htm|ice|int|lgt|lsh|lsr|met|mis|mod|mws|nah|nsh|now|npw|off|omr|paa|pns|pro|psr|rad|rec|rer|rfw|rva|rvr|rvs|rws|sah|sao|saw|scs|sel|ses|sfp|shp|six|sls|sky|smw|sps|stp|sum|svr|svs|swo|swr|swx|sys|taf|tid|tor|trk|tsu|tvl|uvi|wsw|wwa|zfp|wou/){

	@header = ("$gfrom\n", "$news_group\n", "$news_subject\n", "\n");
	filter($finfo, $fpath, $fname, @header);
    }
}

sub filter(){

    my($finfo, $fpath, $fname, @header) = @_;
    my $line;
    my $subline;
    my $status;
    my $nntp;

    if(open(FIN, $fpath)  == undef){
        syslog("err", "Could not open $fpath: $!");
        return;    
    }

    $nntp = Net::NNTP->new($gnewsserver);
    if($nntp == undef){
	close(FIN);
	syslog("err", "Could not open $gnewsserver: $!");
	return;    
    }

    $nntp->post();
    $nntp->datasend(@header);	# includes blank line

    $nntp->datasend("[$finfo]\n");
    read(FIN, $line, 24);	# points past the 24 byte ccb header
    $subline = <FIN>;		# rest of line1 (wmo)
    $nntp->datasend($subline);

    while($line = <FIN>){
	$nntp->datasend($line);
    }
    $nntp->dataend();

    $nntp->quit();
    close(FIN);

    if($gverbose == 1){
	syslog("info", "Feeding $fname to inn.");
    }
}

main();


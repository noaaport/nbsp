#!/usr/local/bin/perl
#
# Copyright (c) 2007 Jose F. Nieves <nieves@ltp.upr.clu.edu>
#
# See LICENSE
#
# $Id$
#
# This is meant to an example of a program that receives PAN messages
# from the panfilter via udp on port 5000.
#
# Each "message" is a string with the following fields:
#
# <wmo header> <awipsid> <file name>
#
# Thus when the awipsid is present in the file, the string has 5 elements
# separated by one space. When there is no awipsid, the string contains
# only four elements (words) separated by a space.
#
# The maximum length of the message is then 62.
#
# wmoheader_size 18
# separator	  1
# awips_size	  6
# separator       1
# fbasename_size 36
#
# The <file name> is the basename of the file. Depending on how
# the spool directory is exported the receiving computer should
# build from that the appropriate path or url to read or request the file.

use Socket;

$message_size = 62;

$port = shift || '5000';
#
# Open datagram socket
#
socket(Server, AF_INET, SOCK_DGRAM, 0) or die "socket: $!";
bind(Server, sockaddr_in($port, INADDR_ANY)) or die "bind: $!";

print "PAN reader started on port $port\n";

#
# Read each PAN message
#
while(1){
    $paddr = recv(Server, $message, $message_size, 0);
    ($port, $iaddr) = sockaddr_in($paddr);
    $name = gethostbyaddr($iaddr, AF_INET);

    @fields = split(/\s+/, $message);

    $wmoid = $fields[0];
    $station = $fields[1];
    $wmotime = $fields[2];
    if($#fields == 4) {
	$awips = $fields[3];
	$fname = $fields[4];
    } else {
	$fname = $fields[3];
    }
    
    print "$wmoid $fname\n";
#    system("wget http://<server>:8015/_get/spool/$fname");
}

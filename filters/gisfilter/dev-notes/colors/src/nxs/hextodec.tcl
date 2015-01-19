#!/usr/bin/tclsh
#
# Converts the n?s color table from hex to decimal.
#

set clist {00 00 00
00 e0 ff
00 80 ff
32 00 96
00 fb 90
00 bb 00
00 8f 00
cd c0 9f
76 76 76
f8 87 00
ff cf 00
ff ff 00
ae 00 00
d0 70 00
ff 00 00
77 00 7d};

foreach t [string trim [split $clist "\n"]] {
    scan $t "%x %x %x" r g b;
    puts "$t: $r $g $b";
}

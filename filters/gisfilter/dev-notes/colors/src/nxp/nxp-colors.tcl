#!/usr/bin/tclsh
#
# Converts the n1p color table from hex to decimal.
#

set clist {00 00 00
aa aa aa
76 76 76
00 ff ff
00 af af
00 ff 00
00 8f 00
ff 00 ff
af 32 7d
00 00 ff
32 00 96
ff ff 00
ff aa 00
ff 00 00
ae 00 00
66 00 33};

foreach t [string trim [split $clist "\n"]] {
    scan $t "%x %x %x" r g b;
    puts "$r $g $b";
}

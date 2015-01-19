#!/usr/bin/tclsh

set body {0, ND,    ND,              00 00 00, black, SNR<TH
1, -50,   v <= -50,        00 E0 FF, light blue, -50
2, -40,  -50 < v <= -40,   00 80 FF, medium blue, -40-50
3, -30,  -40 < v <= -30,   32 00 96, dark blue, -30-40
4, -22,  -30 < v <= -22,   00 FB 90, light green, -22-30
5, -10,  -22 < v <= -10,   00 BB 00, medium green, -10-22
6, -5,   -10 < v <= -5,    00 8F 00, dark green, -5-10
7, -1,   -5 < v < 0,       CD C0 9F, light gray, -0-5
8, 0,     0 <= v < 5,      76 76 76, dark gray, +0-5
9, 5,     5 <= v < 10,     F8 87 00, medium orange, +5-10
A, 10,   10 <= v < 22,     FF CF 00, medium yellow, +22,10
B, 22,   22 <= v < 30,     FF FF 00, yellow, +30-22
C, 30,   30 <= v < 40,     AE 00 00, dark red, +30-40
D, 40,   40 <= v < 50,     D0 70 00, medium brown, +40-50
E, 50,   50 <= v,          FF 00 00, bright red, +50
    F, ND,   ND,               77 00 7D, dark purple, RF};

set output_tmpl {<tr>
  <td bgcolor="#$color" width="12"></td>
  <td>$val</td>
</tr>}

set output [list];
foreach line [split $body "\n"] {
    set parts [split $line ","];
    set val [string trim [lindex $parts 5]];
    set color [string trim [lindex $parts 3]];
    set color [string tolower [join [split $color] ""]];
    set output [linsert $output 0 [subst $output_tmpl]]; 
}

puts [join $output "\n"];

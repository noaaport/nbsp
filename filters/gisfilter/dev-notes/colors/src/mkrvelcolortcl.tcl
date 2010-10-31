#!/usr/local/bin/tclsh8.6
#
# Used to make the n0v (vrel) entries in gis-colors.def. The color scale,
# taken from the IDC-2620003N.pdf is:
#
# 0 ND SNR<TH 0 0 0 black
# 1 -64 -64 0 224 255 light-blue
# 2 -50 -50-64 0 128 255 medium-blue
# 3 -36 -36-50 50 0 150 dark-blue
# 4 -26 -26-36 0 251 144 light-green
# 5 -20 -20-26 0 187 153 medium-green
# 6 -10 -10-20 0 143 0 dark-green
# 7 -1 0-10 205 201 159 light-gray
# 8 0 +0-10 118 118 118 dark-gray
# 9 +10 +10-20 248 135 0 medium-orange
# A +20 +20-26 255 207 0 medium-yellow
# B +26 +26-36 255 255 0 yellow
# C +36 +36-50 174 0 0 dark-red
# D +50 +50-64 208 112 0 medium-brown
# E +64 +64 255 0 0 bright-red
# F RF RF 119 0 125 dark-purple
#
# and our scheme will be
#
# start   end    family   max_number num_n0r_levels
# -65     xx     white
# -64	  -36	 blue     256	   29
# -35     -10    green    128	   26
# -9      -1     gray      88       9
#  0       9     beige     48      10
# 10      35     orange   128      26
# 36      64     red      256      29     
# 65	  xx	 magenta
#
set body {set radcolor(rvel,$level) [::nbsp::gis::rgbcolor $family $number];}

puts {set radcolor(rvel,-65) [::nbsp::gis::x11color black];};

puts "";

set family "blue";
set number 1;
set level -64;
set step 8;
while {$level <= -36} {
    puts [subst -nobackslashes -nocommands $body];
    incr level;
    incr number $step;
}

puts "";

set family "green";
set number 1;
set level -35;
set step 4;
while {$level <= -10} {
    puts [subst -nobackslashes -nocommands $body];
    incr level;
    incr number $step;
}

puts "";

set family "gray";
set number 1;
set level -9;
set step 8;
while {$level <= -1} {
    puts [subst -nobackslashes -nocommands $body];
    incr level;
    incr number $step;
}

puts "";

set family "beige";
set number 1;
set level 0;
set step 4;
while {$level <= 9} {
    puts [subst -nobackslashes -nocommands $body];
    incr level;
    incr number $step;
}

puts "";

set family "orange";
set number 1;
set level 10;
set step 4;
while {$level <= 35} {
    puts [subst -nobackslashes -nocommands $body];
    incr level;
    incr number $step;
}

puts "";

puts "";

set family "red";
set number 140;
set level 36;
set step 4;
while {$level <= 64} {
    puts [subst -nobackslashes -nocommands $body];
    incr level;
    incr number $step;
}

puts ""

puts {set radcolor(rvel,65) [::nbsp::gis::rgbcolor magenta 160];};

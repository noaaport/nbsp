#!/usr/local/bin/tclsh8.6
#
# Used to make the n0r (bref) entries in gis-colors.def
#
# start   end    family   max_number num_n0r_levels
# 5	19	blue	256	   15
# 20	34	green	128	   15
# 35	44	yellow	128	   10
# 45	49	orange	128	   5
# 50	64	red	256	   15
# 65	74	magenta	160	   10
# 75	xx	white
#
set body {set radcolor(n0r,$level) [::nbsp::gis::rgbcolor $family $number];}

set family "blue";
set number 1;
set level 5;
set step 17;
while {$level <= 19} {
    puts [subst -nobackslashes -nocommands $body];
    incr level;
    incr number $step;
}

puts "";

set family "green";
set number 1;
set level 20;
set step 8;
while {$level <= 34} {
    puts [subst -nobackslashes -nocommands $body];
    incr level;
    incr number $step;
}

puts "";

set family "yellow";
set number 1;
set level 35;
set step 12;
while {$level <= 44} {
    puts [subst -nobackslashes -nocommands $body];
    incr level;
    incr number $step;
}

puts "";

set family "orange";
set number 1;
set level 45;
set step 25;
while {$level <= 49} {
    puts [subst -nobackslashes -nocommands $body];
    incr level;
    incr number $step;
}

puts "";

set family "red";
set number 1;
set level 50;
set step 17;
while {$level <= 64} {
    puts [subst -nobackslashes -nocommands $body];
    incr level;
    incr number $step;
}

puts "";

set family "magenta";
set number 1;
set level 65;
set step 16;
while {$level <= 74} {
    puts [subst -nobackslashes -nocommands $body];
    incr level;
    incr number $step;
}

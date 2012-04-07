#!/usr/local/bin/tclsh8.4

set awips1 "n0r";
set awips2 "jua";
set awips "n0rjua";
set ymd_hm "20081212_2315";

set s "digatmos/nexrad/nids/%{awips2}/%{awips1}/%{awips}_%{ymd_hm}.nids";

regsub -all {%} $s {$} t;
puts [subst $t];

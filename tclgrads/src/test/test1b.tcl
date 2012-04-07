#!/usr/local/bin/tclsh8.6

#lappend auto_path "/usr/local/lib/nbsp/tcl/grads";
lappend auto_path "../";
package require grads;

#
# main
#
set ctlfile "yq.ctl";

set status [catch {
    grads::init;
    grads::exec open $ctlfile;
} errmsg];
if {$status != 0} {
    puts $errmsg;
    exit 1;
}

set dx 1;
set dy 1;

gradsu::dims sizes;
gradsu::levels levels;
gradsu::times times;

set lon1 $sizes(lon1);
set lon2 $sizes(lon2);
set lat1 $sizes(lat1);
set lat2 $sizes(lat2);

set header "";
foreach key [array names sizes] {
    if {$header eq ""} {
	append header "\#$key=" $sizes($key);
    } else {
	append header ":$key=" $sizes($key);
    }
}


set level [lindex $levels 2];
set time [lindex $times 1];
gradsu::mset z 2 t 2;

append header ":level=" $level ":time=" $time

puts $header;

set lat $lat2;
while {$lat >= $lat1} {
    gradsu::mset lat $lat;

    set lon $lon1;
    while {$lon <= $lon2} {
	gradsu::mset lon $lon;

	gradsu::display tmpprs;
	set t [grads::output_value];

	set u [grads::eval_expr ugrdprs];

	grads::exec d vgrdprs;
	set v [grads::output_value];

	puts "$lon $lat $t,$u,$v";

	set lon [expr $lon + $dx];
    }
    set lat [expr $lat - $dy];
}

grads::end;

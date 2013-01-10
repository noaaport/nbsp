#!/usr/local/bin/tclsh8.6

##lappend auto_path "/usr/local/lib/nbsp/tcl/grads";
lappend auto_path "..";
package require grads;
#source ../grads.tcl

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

grads::get_dimensions dims;
grads::get_levels levels;
grads::get_times times;

set lon1 $dims(lon1);
set lon2 $dims(lon2);
set lat1 $dims(lat1);
set lat2 $dims(lat2);

set header "";
foreach key [array names dims] {
    if {$header eq ""} {
	append header "\#$key=" $dims($key);
    } else {
	append header ":$key=" $dims($key);
    }
}


set level [lindex $levels 2];
set time [lindex $times 1];
grads::exec set z 2;
grads::exec set t 2;

append header ":level=" $level ":time=" $time

puts $header;

set lat $lat2;
while {$lat >= $lat1} {
    grads::exec set lat $lat;

    set lon $lon1;
    while {$lon <= $lon2} {
	grads::exec set lon $lon;

	grads::exec d tmpprs;
	set t [grads::output_value];

	set u [grads::eval_expr ugrdprs];

	gradsu::getval vgrdprs v;

	puts "$lon $lat $t,$u,$v";

	set lon [expr $lon + $dx];
    }
    set lat [expr $lat - $dy];
}

grads::end;

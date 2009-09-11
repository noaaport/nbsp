#!/usr/local/bin/tclsh8.4

package require textutil;

set prslevels [list 1000 900 800 700 600 500 400 300 200 100];
set fh [list 0 6 12 18 24 30 36 42 48 54 60 66 72 78 84 90 96 102 \
	    108 114 120 126 132 138 144 150 156 162 168 174 180 192 \
	    204 216 228 240];


set params(sfc) [list pres];
set params(trop) [list pres tmp ws wd];
set params(prs) [list hgt tmp rh ws wd];

set header "stdid stnm time lat lon";

set numparams 2;	# lat lon
foreach l [list sfc trop] {
    foreach p $params(${l}) {
	set name $p;
	append name "_" ${l};
	puts $name;
	append header " " $name;
	incr numparams;
    }
}

foreach l $prslevels {
    foreach p $params(prs) {
	set name $p;
	append name "_" ${l}mb;
	puts $name;
	append header " " $name;
	incr numparams;
    }
}

puts $header;
puts $numparams;

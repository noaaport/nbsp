#!/usr/bin/tclsh

source radstations.tcl;

set s1 "ama bro crp";
set s2 "fdr inx tlx";
set s [list $s1 $s2];
set r [nbsp::radstations::extent_bysitelist $s1 $s2];
puts $r;

set s1 [list ama bro crp];
set s2 [list fdr inx tlx];
set s [list $s1 $s2];
set r [nbsp::radstations::extent_bysitelist [concat $s1 $s2]];
puts $r;

puts "==="
set r [nbsp::radstations::extent_bysitelist $s1 $s2];
puts $r;


set s1 "tx ok";
set s2 "pa ny";
set s [list $s1 $s2];
set r [nbsp::radstations::extent_bystate $s1 $s2];
puts $r;

set s1 [list tx ok];
set s2 [list pa ny];
set s [list $s1 $s2];
set r [nbsp::radstations::extent_bystate [concat $s1 $s2]];
puts $r;

set r [nbsp::radstations::extent_bystate -s 6 [concat $s1 $s2]];
puts $r;

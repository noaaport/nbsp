#!%TCLSH%

source ../src/ssh.tcl

::ssh::connect -t %TCLSHT% -- diablo
::ssh::push diablo {after 5000; puts [exec uname -m]};
::ssh::send diablo;

puts "Waiting for diablo";
::ssh::pop_line diablo line;
puts $line;
::ssh::disconnect diablo;

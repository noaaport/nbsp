#!%TCLSH%

source ../src/ssh.tcl

::ssh::connect -t %TCLSHT% -- diablo
foreach c [list {puts [exec uname -m]} {puts [info hostname]}] {
    ::ssh::push diablo $c;
    ::ssh::send diablo;
    ::ssh::pop_line diablo line;
    puts $line;
}
::ssh::disconnect diablo;

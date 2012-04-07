#!%TCLSH%

source ../src/ssh.tcl

::ssh::connect -t %TCLSHT% -- diablo
foreach c [list {puts [exec uname -m]} {puts [info hostname]}] {
    ::ssh::push diablo $c;
}
::ssh::send_exit diablo;
ssh::pop_all diablo output;
puts $output;
::ssh::disconnect diablo;
